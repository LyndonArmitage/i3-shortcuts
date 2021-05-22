#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

FILE * find_config(char * override);
void parse_config(FILE * config);

int main(int argc, char ** args) {

  char * override_path = NULL;
  if(argc > 1) {
    override_path = args[1];
  }
  FILE * config = find_config(override_path);

  if(config == NULL) {
    fprintf(stderr, "Could not find config file");
    return 1;
  }

  printf("Found config");
  parse_config(config);

  fclose(config);
  return 0;
}

FILE * find_config(char * override) {
  if(override != NULL) {
    return fopen(override, "r");
  }

  FILE * found = NULL;
  char * home_dir = getenv("HOME");

  if(home_dir != NULL) {

    char * path = NULL;
    size_t home_len = strlen(home_dir);

    // Try: $HOME/.config/i3/config
    char * xdg_path = "/.config/i3/config";
    path = malloc(home_len + strlen(xdg_path) + 1);
    if(path == NULL) {
      fprintf(stderr, "Could not allocate path");
      return NULL;
    }

    strcpy(path, home_dir);
    strcat(path, xdg_path);
    found = fopen(path, "r");
    
    if(found != NULL) {
      printf("Found config: %s\n", path);
      free(path);
      return found;
    }

    
    // Try: $HOME/.i3/config
    char * i3_path = "/.i3/config";
    path = realloc(path, home_len + strlen(i3_path) + 1);
    if(path == NULL) {
      fprintf(stderr, "Could not reallocate path");
      return NULL;
    }
    
    strcpy(path, home_dir);
    strcat(path, xdg_path);
    found = fopen(path, "r");

    if(found != NULL) {
      printf("Found config: %s\n", path);
      free(path);
      return found;
    }

    free(path);
  } else {
    fprintf(stderr, "No HOME directory\n");
  }


  // Try: /etc/i3/config
  char * etc_config = "/etc/i3/config";
  found = fopen(etc_config, "r");
  if(found != NULL) {
    printf("Found config: %s\n", etc_config);
  }
  
  return found;
}

void parse_config(FILE * config) {

  char * line = NULL;
  size_t len = 0;
  ssize_t read;
  bool in_bar = false;
  int mode_depth = 0;
  while((read = getline(&line, &len, config)) != -1) {
    if(len <= 0) continue;
    //printf("%s", line);
    if(line[0] == '#') continue;
    if(mode_depth > 0 || in_bar) {
      // handle being in a mode
      // Basically find the closing } or another opening mode
      if(strncmp("mode", line, strlen("mode")) == 0) {
        mode_depth ++;
        // in a mode definition
      }
    if(line[0] == '}') {
      if(in_bar) {
        in_bar = false;
      } else {
        mode_depth --;
        if(mode_depth < 0) mode_depth = 0;
      }
    }
    } else {

      if(strncmp("mode", line, strlen("mode")) == 0) {
        mode_depth ++;
        // in a mode definition
        continue;
      }
      if(strncmp("bar {", line, strlen("bar {")) == 0) {
        in_bar = true;
        continue;
      }

      char * found = strstr(line, "bindsym");
      if(found == NULL) continue;
      if(line[strlen(line)-2] == '\\') {
        // contains more code on next lines
        continue;
      }
      printf("%s", line);
    }
  }

}
