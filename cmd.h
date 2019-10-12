#ifndef CMD_H
#define CMD_H

#define MAX_ARGS 16

typedef struct cmd{
  char *exec;
  char **args;
  char *infile;
  char *outfile;
  bool input;
  bool output;
  bool error;
  char *line;
  int retval;
  struct cmd *next;
} cmd;

cmd* cmd_create()
{
  cmd *result = malloc(sizeof(cmd));
    
  result->exec = NULL; //(char*)malloc(strlen(MAX_ARGS) * sizeof(char));
  result->args = (char**)malloc(MAX_ARGS * sizeof(char*));
  for (int i = 0; i < MAX_ARGS; i++)
  {
    result->args[i] = NULL;//(char*)malloc(strlen(line[i]) * sizeof(char));
  }
  
  result->infile = NULL; //(char*)malloc(strlen(name) * sizeof(char));
  result->outfile = NULL;
  result->input = false;
  result->output = false;
  result->error = false;
  result->line = NULL; // used to print the full line when errors occur
  result->retval = 0;

  return result;
}

void cmd_destroy(cmd *j)
{
  for (int i = 0; i < MAX_ARGS; i++)
  {
    if (j->args[i] != NULL)
      free(j->args[i]);
  }
  
  free(j->args);
  
  if (j->exec != NULL)
    free(j->exec);
  
  if (j->infile != NULL)
    free(j->infile);
  
  if (j->outfile != NULL)
    free(j->outfile);

  if (j->next != NULL)
    cmd_destroy(j->next);

  free(j);
}
// void cmd_link_free(cmd *head)
// {
//   cmd *p;
//   while (head)
//   {
//     p = head;
//     head = head->next;
//     free(p);
//   }
// }

void cmd_setRetval(cmd *j, int num)
{
  j->retval = num;
}

void cmd_setExec(cmd *j, char *cmd)
{
  j->exec = (char*)malloc(strlen(cmd) * sizeof(char));
  strcpy(j->exec, cmd);
 
  if (!isalpha(cmd[0]) && cmd[0] != '.')
  {  
    j->error = true;
    fprintf(stderr, "Error: missing command\n");
  }
}

void cmd_addArg(cmd *j, char *a)
{
  for (int i = 0; i < MAX_ARGS; i++)
  {
    if (j->args[i] == NULL)
    {
      j->args[i] = (char*)malloc(strlen(a) * sizeof(char));
      strcpy(j->args[i], a);
      return;
    }
  }

  fprintf(stderr, "Error: too many process arguments\n");
  j->error = true;

}

void cmd_setInFile(cmd *j, char *name)
{
  if (strlen(name) == 0)
  {
    j->error = true;
    fprintf(stderr, "Error: no input file\n");
    return;
  }

  j->infile = (char*)malloc(strlen(name) * sizeof(char));
  strcpy(j->infile, name);

  if (access(name, F_OK) == -1)
  {
    j->error = true;
    fprintf(stderr, "Error: cannot open input file\n");
  }

}

void cmd_setOutFile(cmd *j, char *name)
{
  if (strlen(name) == 0)
  {
    j->error = true;
    fprintf(stderr, "Error: no output file\n");
    return;
  }

  j->outfile = (char*)malloc(strlen(name) * sizeof(char));
  strcpy(j->outfile, name);
  
  if (access(name, F_OK) == -1)
  {
    j->error = true;
    fprintf(stderr, "Error: cannot open output file\n");
  }
}

void cmd_setIn(cmd *j, bool in)
{
  j->input = in;
}

void cmd_setOut(cmd *j, bool out)
{
  j->output = out;
}

//checking for mislocated < > & errors
void cmd_checkError(cmd *j,char *line){
  bool p = false;
  bool o = false;
  for (int i = 0; i < strlen(line); i++)
  {
    if ((line[i] == '&') && (i != strlen(line) - 1))
    {
      j->error = true;
      fprintf(stderr, "Error: mislocated background sign\n");
      break;
    }

    if(line[i] == '|')
    {
      p = true;
      if (o)
      {
        j->error = true;
	fprintf(stderr, "Error: mislocated output redirection\n");
        break;
      }
    }
    else if(line[i] == '>')
    {
      o = true;
    }
    else if (line[i] == '<')
    {
      if (p)
      {
        j->error = true;
	fprintf(stderr, "Error: mislocated input redirection\n");
        break;
      }
    }
  }

}

void cmd_setLine(cmd *j, char *line)
{
  j->line = (char*)malloc(strlen(line) * sizeof(char));
  strcpy(j->line, line);
  
  //if (!isalpha(line[0]) && line[0] != '.')
  //{  
  //  j->error = true;
  //  fprintf(stderr, "2Error: missing command\n");
  //}
}


#endif 
