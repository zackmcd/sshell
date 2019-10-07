#ifndef JOB_H
#define JOB_H

#define MAX_ARGS 16

typedef struct {
  char *exec;
  char **args;
  char *file;
  bool input;
  bool output;
  bool error;
  char *line;
} job;

job* job_create()
{
  job *result = malloc(sizeof(job));
    
  result->exec = NULL; //(char*)malloc(strlen(MAX_ARGS) * sizeof(char));
  result->args = (char**)malloc(MAX_ARGS * sizeof(char*));
  for (int i = 0; i < MAX_ARGS; i++)
  {
    result->args[i] = NULL;//(char*)malloc(strlen(line[i]) * sizeof(char));
  }
  
  result->file = NULL; //(char*)malloc(strlen(name) * sizeof(char));
  result->input = false;
  result->output = false;
  result->error = false;
  result->line = NULL; // used to print the full line when errors occur

  return result;
}

void job_destroy(job *j)
{
  for (int i = 0; i < MAX_ARGS; i++)
  {
    if (j->args[i] != NULL)
      free(j->args[i]);
  }
  
  free(j->args);
  
  if (j->exec != NULL)
    free(j->exec);
  
  if (j->file != NULL)
    free(j->file);
  
  free(j);
}

void job_setExec(job *j, char *cmd)
{
  j->exec = (char*)malloc(strlen(cmd) * sizeof(char));
  strcpy(j->exec, cmd);
}

void job_addArg(job *j, char *a)
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

void job_setFile(job *j, char *name)
{
  j->file = (char*)malloc(strlen(name) * sizeof(char));
  strcpy(j->file, name);
}

void job_setIn(job *j, bool in)
{
  j->input = in;
}

void job_setOut(job *j, bool out)
{
  j->output = out;
}

void job_setLine(job *j, char *line)
{
  j->line = (char*)malloc(strlen(line) * sizeof(char));
  strcpy(j->line, line);
}

#endif 
