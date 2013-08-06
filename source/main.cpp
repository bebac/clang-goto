// ----------------------------------------------------------------------------
#include <program_options.h>
// ----------------------------------------------------------------------------
#include <clang-c/Index.h>
#include <clang-c/CXCompilationDatabase.h>
// ----------------------------------------------------------------------------
#include <iostream>
#include <iomanip>
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
        //compilation_db("."),
        location(),
        check(false)
    {
        add('h', "help", "display this message", help);
        //add('d', "compilation-db", "compilation database file", compilation_db, "DIR");
        add('l', "location", "<filename>:<line>:<column> of a c/c++ translation unit", location, "FILE");
        add(-1,  "check", "parse location filename and print clang diagnostics", check);
    }
public:
    bool        help;
    //std::string compilation_db;
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
        << "Usage: example [OPTION...]" << std::endl
        << std::endl
        << "Example application" << std::endl
        << std::endl
        << "Available option:" << std::endl
        << options << std::endl;
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
        std::cout << err.what() << std::endl
                  << "try: example --help" << std::endl;
        exit(1);
    }

    try
    {
        CXIndex index = clang_createIndex( 0, 0 );

        const char* args[] = { "-ObjC++", "-std=c++11", "-Ivendor/program-options/include" };
        //const char* args[] = { "-std=c++11", "-Ivendor/program-options/include" };

        std::tuple<std::string, unsigned, unsigned> location = decode_location(options.location);

        CXTranslationUnit tu;
        tu = clang_parseTranslationUnit(index, std::get<0>(location).c_str(), args, 3, 0, 0, 0);

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

        CXFile file = clang_getFile(tu, std::get<0>(location).c_str());
        CXSourceLocation loc = clang_getLocation(tu, file, std::get<1>(location), std::get<2>(location));
        CXCursor cursor = clang_getCursor(tu, loc);

        if ( clang_Cursor_isNull(cursor) ) {
            throw std::runtime_error("invalid source location");
        }

        //CXString info = clang_getCursorUSR(cursor);
        CXString info = clang_getCursorDisplayName(cursor);
        CXCursorKind kind = clang_getCursorKind(cursor);

        std::cerr << "cursor kind=" << kind << " is definition " << clang_isCursorDefinition(cursor) << std::endl;

        //CXType type = clang_getCursorType(cursor);

        //std::cout << "kind=" << kind << ", info=" << clang_getCString(info) << std::endl;

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

        CXSourceLocation res_loc = clang_getCursorLocation(res_cursor);
        print_location(res_loc);

        //std::cout << clang_getCursorKind(cursor) << " " << type.kind << " " << clang_getCString(info) << std::endl;
        clang_disposeString(info);
        exit(0);
    }
    catch(const std::exception& err)
    {
        std::cout << err.what() << std::endl;
        exit(1);
    }
}