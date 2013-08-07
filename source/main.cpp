// ----------------------------------------------------------------------------
#include <program_options.h>
#include <json/json.h>
// ----------------------------------------------------------------------------
#include <clang-c/Index.h>
#include <clang-c/CXCompilationDatabase.h>
// ----------------------------------------------------------------------------
#include <iostream>
#include <iomanip>
#include <fstream>
#include <iterator>
#include <string>
#include <stdexcept>

// ----------------------------------------------------------------------------
class options : public program_options::container
{
public:
    options()
        :
        help(false),
        config(".clang-goto"),
        location(),
        check(false)
    {
        add('h', "help", "display this message", help);
        add('c', "config", "json configuration file, default is .clang-goto", config, "FILE");
        add('l', "location", "<filename>:<line>:<column> of a c/c++ translation unit\n"
                 "F.ex. source/main.cpp:12:8", location, "FILE");
        add(-1,  "check", "parse location <filename> and print clang diagnostics.\n"
                 "Can f.ex. be used to check that all includes are specified\n"
                 "Note: No lookup is performed.", check);
    }
public:
    bool        help;
    std::string config;
    std::string location;
    bool        check;
};


// ----------------------------------------------------------------------------
#if 0
CXChildVisitResult cursor_visit(CXCursor cursor, CXCursor parent, CXClientData client_data)
{
    CXString     info     = clang_getCursorDisplayName(cursor);
    CXCursorKind kind     = clang_getCursorKind(cursor);
    CXType       type     = clang_getCursorType(cursor);
    CXString     kind_str = clang_getTypeKindSpelling(type.kind);

    std::cout << "kind=" << std::setw(3) << kind << " [" << std::setw(25) << clang_getCString(kind_str) << "]" << ", info=" << clang_getCString(info) << std::endl;

    clang_disposeString(info);
    clang_disposeString(kind_str);

    return CXChildVisit_Recurse;
}

// ----------------------------------------------------------------------------
void dump_tu(const CXTranslationUnit& tu)
{
    CXCursor cursor = clang_getTranslationUnitCursor(tu);
    clang_visitChildren(cursor, cursor_visit, 0);
}
#endif

// ----------------------------------------------------------------------------
void print_usage(const options& options)
{
    std::cout
        << "Usage: clang-goto [OPTION...]" << std::endl
        << std::endl
        << "clang-goto application. Given a source file location, it will" << std::endl
        << "take you to the definition/declaration of that location." << std::endl
        << std::endl
        << "Available option:" << std::endl
        << options << std::endl;
}

// ----------------------------------------------------------------------------
json::object read_json_configuration(const std::string& filename)
{
    std::ifstream file(filename);

    std::string s((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    json::value  v;
    json::parser parser(v);

    parser.parse(s.c_str(), s.length());

    if ( !v.is_object() ) {
        throw std::runtime_error("json configuration must be an object");
    }

    return std::move(v.as_object());
}

// ----------------------------------------------------------------------------
std::tuple<std::string, unsigned, unsigned> decode_location(const std::string& location)
{
    std::tuple<std::string, unsigned, unsigned> res;

    std::string& filename = std::get<0>(res);
    std::size_t  pos;

    filename = location;

    if ( (pos=filename.rfind(":")) == std::string::npos ) {
        throw std::runtime_error("invalid source location");
    }

    std::get<2>(res) = std::stoul(location.substr(pos+1));
    filename = filename.substr(0, pos);

    if ( (pos=filename.rfind(":")) == std::string::npos ) {
        throw std::runtime_error("invalid source location");
    }

    std::get<1>(res) = std::stoul(location.substr(pos+1));
    filename = filename.substr(0, pos);

    return std::move(res);
}

// ----------------------------------------------------------------------------
void print_location(const CXSourceLocation& location)
{
    CXFile   file;
    CXString filename;
    unsigned line;
    unsigned column;
    unsigned offset;

    clang_getExpansionLocation(location, &file, &line, &column, &offset);

    filename = clang_getFileName(file);

    std::cout << clang_getCString(filename) << ":" << line << ":" << column << std::endl;

    clang_disposeString(filename);
}

// ----------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    options options;

    try
    {
        options.parse(argc, argv);

        if ( options.help )
        {
            print_usage(options);
            exit(1);
        }
    }
    catch(const program_options::error& err)
    {
        std::cout << err.what() << std::endl << "try: clang-goto --help" << std::endl;
        exit(1);
    }

    try
    {
        CXIndex index = clang_createIndex( 0, 0 );

        std::tuple<std::string, unsigned, unsigned> location = decode_location(options.location);

        auto config = read_json_configuration(options.config);
        auto json_args = config["args"];

        //
        // Convert json args to a const char* array we can pass to parseTranslationUnit.
        //
        std::vector<const char*> args;

        if ( !json_args.is_null() )
        {
            if ( !json_args.is_array() ) {
                throw std::runtime_error("json configuration \"args\" must be an array");
            }

            for ( auto& arg : json_args.as_array() ) {
                args.push_back(arg.as_string().c_str());
            }
        }
        else {
            std::cerr << "WARNING - json configuration contains no \"args\" member" << std::endl;
        }

        CXTranslationUnit tu;
        tu = clang_parseTranslationUnit(index, std::get<0>(location).c_str(), &args[0], args.size(), 0, 0, 0);

        if ( options.check )
        {
            for (unsigned i = 0, n = clang_getNumDiagnostics(tu); i != n; ++i)
            {
                CXDiagnostic diagnostic = clang_getDiagnostic(tu, i);
                CXString s = clang_formatDiagnostic(diagnostic, clang_defaultDiagnosticDisplayOptions());

                std::cerr << clang_getCString(s) << std::endl;

                clang_disposeString(s);
            }
            exit(1);
        }

        std::cerr << "filename=" << std::get<0>(location) << ", line=" << std::get<1>(location) << ", column=" << std::get<2>(location) << std::endl;

        // Get a cursor to the given location.
        CXFile file = clang_getFile(tu, std::get<0>(location).c_str());
        CXSourceLocation loc = clang_getLocation(tu, file, std::get<1>(location), std::get<2>(location));
        CXCursor cursor = clang_getCursor(tu, loc);

        if ( clang_Cursor_isNull(cursor) ) {
            throw std::runtime_error("invalid source location");
        }

#if 0
        //CXString info = clang_getCursorUSR(cursor);
        CXString info = clang_getCursorDisplayName(cursor);
        CXCursorKind kind = clang_getCursorKind(cursor);

        std::cerr << "cursor kind=" << kind << " is definition " << clang_isCursorDefinition(cursor) << std::endl;

        //CXType type = clang_getCursorType(cursor);

        //std::cout << "kind=" << kind << ", info=" << clang_getCString(info) << std::endl;

        clang_disposeString(info);
#endif

        //
        // Now do the lookup.
        //
        // NOTE: Not sure what the best approach is here. First trying get-definition then
        //       get-referenced seems to do pretty much what I want.
        //

        CXCursor res_cursor;

        res_cursor = clang_getCursorDefinition(cursor);

        if ( clang_Cursor_isNull(res_cursor) ) {
            std::cerr << "get curser definition returned null" << std::endl;
        }

        res_cursor = clang_getCursorReferenced(cursor);

        if ( clang_Cursor_isNull(res_cursor) ) {
            std::cerr << "get curser referenced returned null" << std::endl;
            exit(1);
        }

        // Output the location.
        CXSourceLocation res_loc = clang_getCursorLocation(res_cursor);
        print_location(res_loc);

        // Cleanup.
        clang_disposeIndex(index);
        clang_disposeTranslationUnit(tu);

        exit(0);
    }
    catch(const std::exception& err)
    {
        std::cout << err.what() << std::endl;
        exit(1);
    }
}