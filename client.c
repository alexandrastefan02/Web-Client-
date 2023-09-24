#include <stdio.h>

#include <stdlib.h>

#include <unistd.h>

#include <string.h>

#include <sys/socket.h>

#include <netinet/in.h>

#include <netdb.h>

#include <arpa/inet.h>

#include "helpers.h"

#include "requests.h"

#include "parson.c"

char * extractToken(const char * json) {
  JSON_Value * root = json_parse_string(json);
  if (root != NULL) {
    JSON_Object * object = json_value_get_object(root);
    const char * token = json_object_get_string(object, "token");
    if (token != NULL) {
      char * tokenCopy = strdup(token);
      json_value_free(root);
      return tokenCopy;
    }
    json_value_free(root);
  }
  return NULL;
}
char * getFirstLine(const char * inputString) {
  size_t length = 0;
  while (inputString[length] != '\0' && inputString[length] != '\n') {
    length++;
  }

  char * firstLine = (char * ) malloc(length + 1);
  if (firstLine == NULL) {
    printf("Memory allocation failed.\n");
    return NULL;
  }

  for (size_t i = 0; i < length; i++) {
    firstLine[i] = inputString[i];
  }
  firstLine[length] = '\0';

  return firstLine;
}
char * extractErrorCode(const char * sentence) {
  char * copy = strdup(sentence);
  char * token = strtok(copy, " \t");
  token = strtok(NULL, " \t");

  if (token != NULL) {
    char * secondWord = strdup(token);
    free(copy);
    return secondWord;
  }

  free(copy);
  return NULL;
}
JSON_Value * extractAndParseJSON(const char * inputString) {
  const char * jsonStart = strchr(inputString, '{');
  if (jsonStart == NULL) {
    //printf("Failed to find the start of JSON substring.\n");
    return NULL;
  }

  const char * jsonEnd = strrchr(inputString, '}');
  if (jsonEnd == NULL) {
    //printf("Failed to find the end of JSON substring.\n");
    return NULL;
  }

  size_t jsonLength = jsonEnd - jsonStart + 1;

  char * jsonString = (char * ) malloc(jsonLength + 1);
  if (jsonString == NULL) {
    //printf("Memory allocation failed.\n");
    return NULL;
  }

  strncpy(jsonString, jsonStart, jsonLength);
  jsonString[jsonLength] = '\0';

  JSON_Value * rootValue = json_parse_string(jsonString);
  if (rootValue == NULL) {
    //printf("Failed to parse JSON string.\n");
    free(jsonString);
    return NULL;
  }

  free(jsonString);
  return rootValue;
}
char * extractCookieValue(const char * inputString) {
  const char * cookiePrefix = "Set-Cookie: ";
  const char * cookieSuffix = ";";

  const char * start = strstr(inputString, cookiePrefix);
  if (start == NULL) {
    //printf("Failed to find the start of the cookie value.\n");
    return NULL;
  }
  start += strlen(cookiePrefix);

  const char * end = strstr(start, cookieSuffix);
  if (end == NULL) {
    //printf("Failed to find the end of the cookie value.\n");
    return NULL;
  }

  size_t cookieLength = end - start;
  char * cookieValue = (char * ) malloc(cookieLength + 1);
  if (cookieValue == NULL) {
    // printf("Memory allocation failed.\n");
    return NULL;
  }

  strncpy(cookieValue, start, cookieLength);
  cookieValue[cookieLength] = '\0';

  return cookieValue;
}

int main(int argc, char * argv[]) {
  char * authProof;
  char * message = calloc(10000, sizeof(char));
  char * response;
  int sockfd;
  char * token;
  char command[10000];
  while (1) {
    sockfd = open_connection("34.254.242.81", 8080, AF_INET, SOCK_STREAM, 0);
    scanf("%s", command);
    if (strcmp(command, "register") == 0) {
      printf("username= ");
      char * username, * password;
      username = malloc(200);
      password = malloc(200);
      scanf("%s", username);
      printf("password= ");
      scanf("%s", password);
      //TODO:invalidate if they have space!!!!!!!!!!!
      JSON_Value * root_value = json_value_init_object();
      JSON_Object * root_object = json_value_get_object(root_value);
      char * serialized_string = NULL;
      json_object_set_string(root_object, "username", username);
      json_object_set_string(root_object, "password", password);
      serialized_string = json_serialize_to_string_pretty(root_value);
      // puts(serialized_string);
    //   sockfd = open_connection("34.254.242.81", 8080, AF_INET, SOCK_STREAM, 0);
      message = compute_post_request("34.254.242.81", "/api/v1/tema/auth/register", "application/json", serialized_string, 1, NULL, 0, NULL);
      send_to_server(sockfd, message);
      response = receive_from_server(sockfd);
      JSON_Value * possibleErrorRoot = extractAndParseJSON(response);
      if (possibleErrorRoot == NULL) {
        printf("200 - User registered successfully!\n");
      } else {
        printf("User already exists!\n");
      }
      json_free_serialized_string(username);
      json_free_serialized_string(password);

    }
    if (strcmp(command, "login") == 0) {
      printf("username= ");
      char * username, * password;
      username = malloc(200);
      password = malloc(200);
      scanf("%s", username);
      printf("password= ");
      scanf("%s", password);
      JSON_Value * root_value = json_value_init_object();
      JSON_Object * root_object = json_value_get_object(root_value);
      char * serialized_string = NULL;
      json_object_set_string(root_object, "username", username);
      json_object_set_string(root_object, "password", password);
      serialized_string = json_serialize_to_string_pretty(root_value);
     // sockfd = open_connection("34.254.242.81", 8080, AF_INET, SOCK_STREAM, 0);
      message = compute_post_request("34.254.242.81", "/api/v1/tema/auth/login", "application/json", serialized_string, 1, NULL, 0, NULL);
      send_to_server(sockfd, message);
      char * response;
      response = receive_from_server(sockfd);
      JSON_Value * possibleErrorRoot = extractAndParseJSON(response);
      if (possibleErrorRoot == NULL) {
        printf("200 - You are logged in!\n");
        authProof = extractCookieValue(response);
      } else {
        printf("Wrong creditentials!\n");
      }
      json_free_serialized_string(username);
      json_free_serialized_string(password);
    }
    if (strcmp(command, "enter_library") == 0) {
      message = compute_get_request("34.254.242.81", "/api/v1/tema/library/access", NULL, authProof, 1, NULL);
     // sockfd = open_connection("34.254.242.81", 8080, AF_INET, SOCK_STREAM, 0);
      send_to_server(sockfd, message);
      response = receive_from_server(sockfd);
      char * line;
      line = getFirstLine(response);
      char * errorCode = extractErrorCode(line);
      if (strcmp(errorCode, "401") == 0) {
        printf("You are not authorized to enter the library!\n");
      } else if (strcmp(errorCode, "200") == 0) {
        printf("Valid Entry!\n");
        JSON_Value * jsonToken = extractAndParseJSON(response);
        char * jsonString = json_serialize_to_string(jsonToken);
        token = extractToken(jsonString);
      }
      close(sockfd);
    }
    if (strcmp(command, "add_book") == 0) {
      char * title, * author, * genre, * publisher;
      int page_count;
      title = malloc(20);
      author = malloc(20);
      genre = malloc(20);
      publisher = malloc(20);
      if (token == NULL) {
        printf("Please enter the library first!\n");
      } else {
        printf("title= ");
        scanf("%s", title);
        printf("author= ");
        scanf("%s", author);
        printf("genre= ");
        scanf("%s", genre);
        printf("publisher= ");
        scanf("%s", publisher);
        printf("page_count= ");
        int result = scanf("%d", & page_count);
        if (result != 1) {
          printf("Invalid number of pages!\n");
        } else
        if (strlen(title) == 0 || strlen(author) == 0 || strlen(genre) == 0 || strlen(publisher) == 0) {
          printf("Please complete all the fields!\n");

        } else {
          JSON_Value * root_value = json_value_init_object();
          JSON_Object * root_object = json_value_get_object(root_value);
          char * serialized_string = NULL;
          json_object_set_string(root_object, "title", title);
          json_object_set_string(root_object, "author", author);
          json_object_set_string(root_object, "genre", genre);
          json_object_set_number(root_object, "page_count", page_count);
          json_object_set_string(root_object, "publisher", publisher);

          serialized_string = json_serialize_to_string_pretty(root_value);
          message = compute_post_request("34.254.242.81", "/api/v1/tema/library/books", "application/json", serialized_string, 1, authProof, 1, token);
         // sockfd = open_connection("34.254.242.81", 8080, AF_INET, SOCK_STREAM, 0);
          send_to_server(sockfd, message);
          response = receive_from_server(sockfd);
          printf("%s", response);
          char * line;
          line = getFirstLine(response);
          char * errorCode = extractErrorCode(line);
          if (strcmp(errorCode, "200") == 0) {
            printf("Book added succesfully!");
          }

        }
      }
      json_free_serialized_string(title);
      json_free_serialized_string(author);
      json_free_serialized_string(genre);
      json_free_serialized_string(publisher);
    }
    if (strcmp(command, "get_books") == 0) {
      message = compute_get_request("34.254.242.81", "/api/v1/tema/library/books", NULL, authProof, 1, token);
     // sockfd = open_connection("34.254.242.81", 8080, AF_INET, SOCK_STREAM, 0);
      send_to_server(sockfd, message);
      response = receive_from_server(sockfd);
      char * line;
      line = getFirstLine(response);
      char * errorCode = extractErrorCode(line);
      if (strcmp(errorCode, "403") == 0) {
        printf("Please enter the library first!\n");
      } else {
        printf("Here are your books: \n");
        printf("%s\n", response);
      }

    }
    if (strcmp(command, "delete_book") == 0) {
      char * bookid, * route;
      bookid = malloc(20);
      route = malloc(40);
      memcpy(route, "/api/v1/tema/library/books/", 28);
      printf("id= ");
      scanf("%s", bookid);
      strcat(route, bookid);
      message = compute_delete_request("34.254.242.81", route, NULL, authProof, 1, token);
     // sockfd = open_connection("34.254.242.81", 8080, AF_INET, SOCK_STREAM, 0);
      send_to_server(sockfd, message);
      response = receive_from_server(sockfd);
      char * line;
      line = getFirstLine(response);
      char * errorCode = extractErrorCode(line);
      if (strcmp(errorCode, "404") == 0) {
        printf("Book not found!\n");
      } else if (strcmp(errorCode, "403") == 0) {
        printf("Please enter the library first!\n");
      } else if (strcmp(errorCode, "500") == 0) {
        printf("Invalid book id\n");
      } else {
        printf("Book deleted \n");

      }
    }
    if ((strcmp(command, "get_book_id") == 0)) {
      char * bookid, * route;
      bookid = malloc(20);
      route = malloc(40);
      memcpy(route, "/api/v1/tema/library/books/", 28);
      printf("id= ");
      scanf("%s", bookid);
      strcat(route, bookid);
      message = compute_get_request("34.254.242.81", route, NULL, authProof, 1, token);
      //sockfd = open_connection("34.254.242.81", 8080, AF_INET, SOCK_STREAM, 0);
      send_to_server(sockfd, message);
      response = receive_from_server(sockfd);
      char * line;
      line = getFirstLine(response);
      char * errorCode = extractErrorCode(line);
      if (strcmp(errorCode, "404") == 0) {
        printf("Book not found!\n");
      } else if (strcmp(errorCode, "403") == 0) {
        printf("Please enter the library first!\n");
      } else if (strcmp(errorCode, "400") == 0) {
        printf("Invalid book id\n");
      } else {
        printf("Here is your book:\n");
        JSON_Value * bookRoot = extractAndParseJSON(response);
        char * book = json_serialize_to_string_pretty(bookRoot);
        printf("%s", book);
      }
    }
    if (strcmp(command, "logout") == 0) {
      message = compute_get_request("34.254.242.81", "/api/v1/tema/auth/logout", NULL, authProof, 1, token);
     // sockfd = open_connection("34.254.242.81", 8080, AF_INET, SOCK_STREAM, 0);
      send_to_server(sockfd, message);
      response = receive_from_server(sockfd);
      char * line;
      line = getFirstLine(response);
      char * errorCode = extractErrorCode(line);
      if (strcmp(errorCode, "400") == 0) {
        printf("You are not logged in!\n");
      } else if (strcmp(errorCode, "200") == 0) {
        printf("You are now logged out!\n");
      }

    }
    if (strcmp(command, "exit") == 0) {
      close_connection(sockfd);
      break;
    }
  }

  return 0;
}