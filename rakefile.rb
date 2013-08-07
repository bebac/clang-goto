# -----------------------------------------------------------------------------
require 'rake/clean'
require 'rake/tasklib'
require 'rbconfig'

# -----------------------------------------------------------------------------
def windows?
    RbConfig::CONFIG['target_os'] =~ /mswin|mingw|cygwin/
end

# -----------------------------------------------------------------------------
if windows?
    require './rakelib/lib/ctasklib-msvc'
else
    require './rakelib/lib/ctasklib'
end

# -----------------------------------------------------------------------------
if windows?
# -----------------------------------------------------------------------------

case ENV["variant"]
when "release"
    ENV["CFLAGS"]  = %q(/c /EHsc /O2 /GA)
    ENV["LDFLAGS"] = %q(/subsystem:console)
else
    ENV["CFLAGS"]  = %q(/c /EHsc /ZI)
    ENV["LDFLAGS"] = %q(/subsystem:console /debug)
end

ENV["CPPFLAGS"] = %q()

# -----------------------------------------------------------------------------
else # gcc
# -----------------------------------------------------------------------------

case ENV["variant"]
when "release"
    ENV["CFLAGS"]  = %q(-O2 -Wall -MMD)
    ENV["LDFLAGS"] = %q(-pthread)
else
    ENV["CFLAGS"]  = %q(-g -Wall -MMD)
    ENV["LDFLAGS"] = %q(-g -pthread)
end

ENV["CPPFLAGS"] = %q(-std=c++11)

end

# -----------------------------------------------------------------------------
popt = Rake::StaticLibraryTask.new("vendor/program-options/program-options.yml")
json = Rake::StaticLibraryTask.new("vendor/json/json.yml")

# -----------------------------------------------------------------------------
spec = Rake::ExecutableSpecification.new do |s|
    s.name = 'clang-goto'
    s.includes.add %w(
        vendor/program-options/include
        vendor/json/include
    )
    s.libincludes.add %w(
        build
    )
    s.sources.add %w(
        source/main.cpp
    )
    s.libraries += [ popt, json ] + %w(clang)
end
# -----------------------------------------------------------------------------
Rake::ExecutableTask.new(:"clang-goto", spec)

# -----------------------------------------------------------------------------
CLEAN.include('build')
# -----------------------------------------------------------------------------
task :default => [ :"clang-goto" ]
task :all => [ :default ]
