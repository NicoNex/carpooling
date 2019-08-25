#ifndef FILEHANDLER_H_
#define FILEHANDLER_H_

#include <json.h>

// Returns the json_object parsed from the file in input.
json_object *load_json_from_file(const char *filepath);


#endif // FILEHANDLER_H_
