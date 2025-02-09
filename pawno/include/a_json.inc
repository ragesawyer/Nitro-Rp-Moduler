#if defined _a_json_included
	#endinput
#endif
#define _a_json_included

#include <a_samp>

#if !defined JSON_DEFAULT_DELIMITER
	#define JSON_DEFAULT_DELIMITER '/'
#endif

#define JSON_NULL 0
#define JSON_STRING 1
#define JSON_NUMBER 2
#define JSON_BOOL 3
#define JSON_ARRAY 4
#define JSON_NODE 5

native JSONNode:json_parse_file(const file_path[]);
native JSONNode:json_parse_string(const string[]);
native json_close(JSONNode:node);
native json_get_type(JSONNode:node, const path[] = '\0', const path_delim = JSON_DEFAULT_DELIMITER);
native JSONNode:json_get_node(JSONNode:node, const path[], const path_delim = JSON_DEFAULT_DELIMITER);
native bool:json_get_bool(JSONNode:node, const path[] = '\0', const path_delim = JSON_DEFAULT_DELIMITER);
native json_get_int(JSONNode:node, const path[] = '\0', const path_delim = JSON_DEFAULT_DELIMITER);
native Float:json_get_float(JSONNode:node, const path[] = '\0', const path_delim = JSON_DEFAULT_DELIMITER);
native json_get_string(JSONNode:node, dst[], const len = sizeof(dst), const path[] = '\0', const bool:packed = false, const path_delim = JSON_DEFAULT_DELIMITER);
native json_get_name(JSONNode:node, dst[], const len = sizeof(dst), const bool:packed = false);

native JSONArray:json_get_array(JSONNode:node, const path[] = '\0', const path_delim = JSON_DEFAULT_DELIMITER);
native json_array_count(JSONArray:array);
native JSONNode:json_array_at(JSONArray:array, const index);
