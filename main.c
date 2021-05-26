#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef struct Config {
  FILE * file;
  char * path;
} Config;

typedef struct Shortcut {
  char * key_combo;
  char * command;
} Shortcut;

Shortcut * new_shortcut(char * line);
void print_Shortcut(Shortcut * shortcut);
void delete_Shortcut(Shortcut * shortcut);

typedef struct ListEntry {
  Shortcut * entry;
  struct ListEntry * next;
} ListEntry;

typedef struct List {
  ListEntry * head;
  int count;
} List;

void delete_List(List * list);

Config * find_config(char * override);
List * parse_config(Config * config);

int main(int argc, char ** args) {

  char * override_path = NULL;
  if(argc > 1) {
    override_path = args[1];
  }
  Config* config = find_config(override_path);

  if(config == NULL) {
    fprintf(stderr, "Could not find config file");
    return 1;
  }

  List * list = parse_config(config);

  ListEntry * entry = list->head;
  while(entry != NULL) {
    Shortcut * shortcut = entry->entry;
    print_Shortcut(shortcut);
    entry = entry->next;
  }
  delete_List(list);

  return 0;
}

Config * find_config(char * override) {
  if(override != NULL) {
    FILE * file = fopen(override, "r");
    if(file == NULL) return NULL;
    Config * config = malloc(sizeof(Config));
    if(config == NULL) {
      fprintf(stderr, "Could not create config");
      exit(1);
    }
    config->path = override;
    config->file = file;
    return config;
  }

  FILE * found = NULL;
  char * home_dir = getenv("HOME");

  Config * config = NULL;

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
      config = malloc(sizeof(Config));
      if(config == NULL) {
        fprintf(stderr, "Could not create config");
        exit(1);
      }
      config->path = path;
      config->file = found;
      return config;
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
      config = malloc(sizeof(Config));
      if(config == NULL) {
        fprintf(stderr, "Could not create config");
        exit(1);
      }
      config->path = path;
      config->file = found;
      return config;
    }

    free(path);
  } else {
    fprintf(stderr, "No HOME directory\n");
  }


  // Try: /etc/i3/config
  char * etc_config = "/etc/i3/config";
  found = fopen(etc_config, "r");
  if(found != NULL) {
    config = malloc(sizeof(Config));
    if(config == NULL) {
      fprintf(stderr, "Could not create config");
      exit(1);
    }
    config->path = etc_config;
    config->file = found;
    return config;
  }
  
  return NULL;
}

List * parse_config(Config * config) {
  if(config == NULL) return NULL;

  char * line = NULL;
  size_t len = 0;
  ssize_t read;
  bool in_bar = false;
  int mode_depth = 0;

  List * list = malloc(sizeof(List));
  if(list == NULL) {
    fprintf(stderr, "Could not create list");
    exit(1);
  }
  list->count = 0;
  list->head = NULL;
  ListEntry * last_entry = NULL;

  while((read = getline(&line, &len, config->file)) != -1) {
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
      Shortcut * shortcut = new_shortcut(line);
      if(shortcut != NULL) {
        ListEntry * entry = malloc(sizeof(ListEntry));
        if(entry == NULL) {
          fprintf(stderr, "Could not create list entry");
          exit(1);
        }
        entry->entry = shortcut;
        entry->next = NULL;
        if(last_entry == NULL) {
          list->head = entry;
        } else {
          last_entry->next = entry;
        }
        list->count ++;
        last_entry = entry;
      }
    }
  }
  fclose(config->file);
  return list;
}

Shortcut * new_shortcut(char * line) {
  if(line == NULL) return NULL;
  if(strstr(line, "bindsym") == NULL) return NULL;

  char * bindsym_token = strtok(line, " ");
  char * shortcut_token = strtok(NULL, " ");
  char * command_token = line + (2 + strlen(shortcut_token) + strlen(bindsym_token));

  Shortcut * shortcut = malloc(sizeof(Shortcut));
  if(shortcut == NULL) { 
    fprintf(stderr, "Could not allocate shortcut");
    exit(1);
  }

  char * key_combo = malloc(strlen(shortcut_token) + 1);
  if(key_combo == NULL) {
    fprintf(stderr, "Could not allocate string");
    exit(1);
  }
  strcpy(key_combo, shortcut_token);
  shortcut->key_combo = key_combo;

  char * command = malloc(strlen(command_token) + 1);
  if(key_combo == NULL) {
    fprintf(stderr, "Could not allocate string");
    exit(1);
  }
  strcpy(command, command_token);
  command[strcspn(command, "\n")] = 0;
  shortcut->command = command;
  return shortcut;
}

void print_Shortcut(Shortcut * shortcut) {
  printf("%s\t%s\n", shortcut->key_combo, shortcut->command);
}

void delete_Shortcut(Shortcut * shortcut) {
  if(shortcut == NULL) return;
  free(shortcut->key_combo);
  free(shortcut->command);
  free(shortcut);
  shortcut->key_combo = NULL;
  shortcut->command = NULL;
  shortcut = NULL;
}

void delete_List(List * list) {
  if(list == NULL) return;
  if(list->count > 0 && list->head != NULL) {
    ListEntry * entry = list->head;
    for(int i = 0; i < list->count; i ++) {
      delete_Shortcut(entry->entry);
      entry = entry->next;
    }
  }
  free(list);
  list = NULL;
}
