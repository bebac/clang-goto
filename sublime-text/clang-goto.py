import sublime, sublime_plugin
import subprocess
import os
import re
import json

# -----------------------------------------------------------------------------
SOURCE_RE = re.compile(".*\.(c|cpp|cxx|h|hpp|hxx)$")

# -----------------------------------------------------------------------------
def config_file_exists():
  project_filename = sublime.active_window().project_file_name()
  if project_filename:
    wd = os.path.dirname(project_filename)
    return os.path.exists(os.path.join(wd, ".clang-goto"))
  else:
    return False

# -----------------------------------------------------------------------------
def goto_file(filename):
  sublime.active_window().open_file(filename, sublime.ENCODED_POSITION)

# -----------------------------------------------------------------------------
class ClangGotoEventListener(sublime_plugin.EventListener):
  def on_query_context(self, view, key, operator, operand, match_all):
    #print("key: %s, filename: %s" % (key, view.file_name()))
    if key == "clang_goto_enabled" and SOURCE_RE.match(view.file_name()):
      if config_file_exists():
        return True
      else:
        return None
    else:
      return None

# -----------------------------------------------------------------------------
class ClangGotoCommand(sublime_plugin.TextCommand):
  def run(self, edit):
    filename = self.view.file_name()
    row, col = self.view.rowcol(self.view.sel()[0].a)
    #print("%s:%d:%d" % (filename, row+1, col))

    settings = sublime.load_settings('clang-goto.sublime-settings')

    wd = os.path.dirname(sublime.active_window().project_file_name())

    ###
    # Use configured exectuable or use the default which assumes clang-goto
    # is in the path.
    executable = settings.get('clang_goto_executable', 'clang-goto')
    #print("executable=%s" % executable)

    loc = subprocess.check_output([executable, "--location=%s:%u:%u" % (filename, row+1, col)], cwd=wd)
    loc = loc.strip()

    if loc:
      goto_file(os.path.join(wd, loc.decode()))
    else:
      # TODO: show some status.
      pass
