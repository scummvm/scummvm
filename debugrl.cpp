#ifdef HAVE_READLINE

#include "debugrl.h"

// A lot of this was ripped straight from the readline fileman.c example.

char* _debugger_commands[] = {
  "help",
  "quit",
  "go",
  "actor",
  "scripts",
  "exit",
  (char *)NULL
};


// forwards decls
char ** scumm_debugger_completion (const char *text, int start, int end);
char * scumm_debugger_command_generator (const char *text, int state);

void initialize_readline () {
  /* Allow conditional parsing of the ~/.inputrc file. */
  rl_readline_name = "scummvm";

  /* Tell the completer that we want a crack first. */
  rl_attempted_completion_function = scumm_debugger_completion;
}

char ** scumm_debugger_completion (const char *text, int start, int end) {

  char **matches;

  matches = (char **)NULL;

  // If this word is at the start of the line, then it is a command
  // to complete.
  if (start == 0) {
    matches = rl_completion_matches (text, scumm_debugger_command_generator);
  } else {
    // At some stage it'd be nice to have symbolic actor name completion
    // or something similarly groovy. Not right now though.
  }

  // This just makes sure that readline doesn't try to use its default
  // completer, which uses filenames in the current dir, if we can't find 
  // a match, since filenames don't have much use in the debuger :)
  // There's probably a nice way to do this once, rather than every time.
  rl_attempted_completion_over = 1;
  
  return (matches);
}


/* Generator function for command completion.  STATE lets us know whether
   to start from scratch; without any state (i.e. STATE == 0), then we
   start at the top of the list. */
char * scumm_debugger_command_generator (const char *text, int state) {

  static int list_index, len;
  char *name;

  /* If this is a new word to complete, initialize now.  This includes
     saving the length of TEXT for efficiency, and initializing the index
     variable to 0. */
  if (!state)
    {
      list_index = 0;
      len = strlen (text);
    }

  /* Return the next name which partially matches from the command list. */
  while (name = _debugger_commands[list_index])
    {
      list_index++;

      if (strncmp (name, text, len) == 0)
        //return (dupstr(name));
        return strdup(name);
    }

  /* If no names matched, then return NULL. */
  return ((char *)NULL);
}

#endif /* HAVE_READLINE */
