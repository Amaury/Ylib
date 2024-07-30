#include "yvar.h"

/* ********** PRIVATE FUNCTIONS ********** */
static yvar_t *_yvar_default_delete(yvar_t *var);
static yvar_t *_yvar_default_clone(yvar_t *var);
static ystatus_t _yvar_delete_table_item(uint64_t index, char *key, void *data, void *user_data);

/* ********** GLOBAL VARIABLES ********** */
static yvar_definition_t _yvar_default_definition_g = {
	.delete_function = _yvar_default_delete,
	.clone_function = _yvar_default_clone,
};

/* ********** FUNCTIONS ********** */
/* Default deletion of a yvar value. */
yvar_t *_yvar_default_delete(yvar_t *var) {
	if (!var)
		return (NULL);
	if (var->type == YVAR_BINARY)
		ybin_delete_data(&var->binary_value);
	else if (var->type == YVAR_STRING)
		ys_free(var->string_value);
	else if (var->type == YVAR_TABLE)
		ytable_free(var->table_value);
	var->type = YVAR_UNDEF;
	return (var);
}
/* Default cloning of a yvar. */
yvar_t *_yvar_default_clone(yvar_t *var) {
	yvar_t *result;
	if (!var)
		return (NULL);
	if (!(result = malloc0(sizeof(yvar_t))))
		return (NULL);
	*result = (yvar_t){
		.refcount = 1,
		.definition = var->definition,
		.user_data = var->user_data,
		.type = var->type,
	};
	switch (var->type) {
		case YVAR_UNDEF:
		case YVAR_NULL:
			break;
		case YVAR_BOOL:
			result->bool_value = var->bool_value;
			break;
		case YVAR_INT:
			result->int_value = var->int_value;
			break;
		case YVAR_FLOAT:
			result->float_value = var->float_value;
			break;
		case YVAR_CONST_BINARY:
			result->binary_value = var->binary_value;
			break;
		case YVAR_BINARY:
			ybin_copy(&var->binary_value, &result->binary_value);
			break;
		case YVAR_CONST_STRING:
			result->const_value = var->const_value;
			break;
		case YVAR_STRING:
			result->string_value = ys_dup(var->string_value);
			break;
		case YVAR_TABLE:
			result->table_value = ytable_clone(var->table_value);
			break;
		case YVAR_POINTER:
		case YVAR_OBJECT:
			result->pointer_value = var->pointer_value;
			break;
	}
	return (result);
}
/* Create a new undefined yvar. */
yvar_t *yvar_new_undef(void) {
	yvar_t *var = yvar_init_undef(malloc0(sizeof(yvar_t)));
	if (var)
		var->refcount = 1;
	return (var);
}
/* Initialize an yvar structure to an undefined value. */
yvar_t *yvar_init_undef(yvar_t *var) {
	if (!var)
		return (NULL);
	*var = (yvar_t){
		.refcount = -1,
		.definition = &_yvar_default_definition_g,
		.type = YVAR_UNDEF,
	};
	return (var);
}
/* Create a new null yvar. */
yvar_t *yvar_new_null(void) {
	yvar_t *var = yvar_init_null(malloc0(sizeof(yvar_t)));
	if (var)
		var->refcount = 1;
	return (var);
}
/* Initialize an yvar structure to a null value. */
yvar_t *yvar_init_null(yvar_t *var) {
	if (!var)
		return (NULL);
	*var = (yvar_t){
		.refcount = -1,
		.definition = &_yvar_default_definition_g,
		.type = YVAR_NULL,
	};
	return (var);
}
/* Create a new boolean yvar. */
yvar_t *yvar_new_bool(bool value) {
	yvar_t *var = yvar_init_bool(malloc0(sizeof(yvar_t)), value);
	if (var)
		var->refcount = 1;
	return (var);
}
/* Initialize a yvar structure with a boolean value. */
yvar_t *yvar_init_bool(yvar_t *var, bool value) {
	if (!var)
		return (NULL);
	*var = (yvar_t){
		.refcount = -1,
		.definition = &_yvar_default_definition_g,
		.type = YVAR_BOOL,
		.bool_value = value,
	};
	return (var);
}
/* Create a new int yvar. */
yvar_t *yvar_new_int(int64_t value) {
	yvar_t *var = yvar_init_int(malloc0(sizeof(yvar_t)), value);
	if (var)
		var->refcount = 1;
	return (var);
}
/* Initialize a yvar structure with an integer value. */
yvar_t *yvar_init_int(yvar_t *var, int64_t value) {
	if (!var)
		return (NULL);
	*var = (yvar_t){
		.refcount = -1,
		.definition = &_yvar_default_definition_g,
		.type = YVAR_INT,
		.int_value = value,
	};
	return (var);
}
/* Create a new floating-point number yvar. */
yvar_t *yvar_new_float(double value) {
	yvar_t *var = yvar_init_float(malloc0(sizeof(yvar_t)), value);
	if (var)
		var->refcount = 1;
	return (var);
}
/* Initialize a yvar structure with a floating-point value. */
yvar_t *yvar_init_float(yvar_t *var, double value) {
	if (!var)
		return (NULL);
	*var = (yvar_t){
		.refcount = -1,
		.definition = &_yvar_default_definition_g,
		.type = YVAR_FLOAT,
		.float_value = value,
	};
	return (var);
}
/* Create a new constant binary yvar. */
yvar_t *yvar_new_const_binary(ybin_t value) {
	yvar_t *var = yvar_init_const_binary(malloc0(sizeof(yvar_t)), value);
	if (var)
		var->refcount = 1;
	return (var);
}
/* Initialize a yvar structure with a constant binary value. */
yvar_t *yvar_init_const_binary(yvar_t *var, ybin_t value) {
	if (!var)
		return (NULL);
	*var = (yvar_t){
		.refcount = -1,
		.definition = &_yvar_default_definition_g,
		.type = YVAR_CONST_BINARY,
		.binary_value = value,
	};
	return (var);
}
/* Create a new binary yvar. */
yvar_t *yvar_new_binary(ybin_t value) {
	yvar_t *var = yvar_init_binary(malloc0(sizeof(yvar_t)), value);
	if (var)
		var->refcount = 1;
	return (var);
}
/* Initialize a yvar structure with a binary value. */
yvar_t *yvar_init_binary(yvar_t *var, ybin_t value) {
	if (!var)
		return (NULL);
	*var = (yvar_t){
		.refcount = -1,
		.definition = &_yvar_default_definition_g,
		.type = YVAR_BINARY,
		.binary_value = value,
	};
	return (var);
}
/* Create a new constant character string value. */
yvar_t *yvar_new_const_string(const char *value) {
	yvar_t *var = yvar_init_const_string(malloc0(sizeof(yvar_t)), value);
	if (var)
		var->refcount = 1;
	return (var);
}
/* Initialize a yvar structure with a constant character string value. */
yvar_t *yvar_init_const_string(yvar_t *var, const char *value) {
	if (!var)
		return (NULL);
	*var = (yvar_t){
		.refcount = -1,
		.definition = &_yvar_default_definition_g,
		.type = YVAR_CONST_STRING,
		.const_value = value,
	};
	return (var);
}
/* Create a new character string yvar. */
yvar_t *yvar_new_string(ystr_t value) {
	yvar_t *var = yvar_init_string(malloc0(sizeof(yvar_t)), value);
	if (var) {
		var->refcount = 1;
		if (!var->string_value) {
			free0(var);
			return (NULL);
		}
	}
	return (var);
}
/* Initialize a yvar structure with a character string value. */
yvar_t *yvar_init_string(yvar_t *var, ystr_t value) {
	if (!var)
		return (NULL);
	*var = (yvar_t){
		.refcount = -1,
		.definition = &_yvar_default_definition_g,
		.type = YVAR_STRING,
		.string_value = value ? value : ys_new(""),
	};
	return (var);
}
/* Create a new table yvar. */
yvar_t *yvar_new_table(ytable_t *value) {
	yvar_t *var = yvar_init_table(malloc0(sizeof(yvar_t)), value);
	if (var) {
		var->refcount = 1;
		if (!var->table_value) {
			free0(var);
			return (NULL);
		}
	}
	return (var);
}
/* Initialize a yvar structure with a table value. */
yvar_t *yvar_init_table(yvar_t *var, ytable_t *value) {
	if (!var)
		return (NULL);
	*var = (yvar_t){
		.refcount = -1,
		.definition = &_yvar_default_definition_g,
		.type = YVAR_TABLE,
		.table_value = value ? value : ytable_new(),
	};
	if (!value)
		ytable_set_delete_function(var->table_value, _yvar_delete_table_item, NULL);
	return (var);
}
/* Create a new pointer yvar. */
yvar_t *yvar_new_pointer(void *value) {
	yvar_t *var = yvar_init_pointer(malloc0(sizeof(yvar_t)), value);
	if (var)
		var->refcount = 1;
	return (var);
}
/* Initialize a yvar structure with a pointer. */
yvar_t *yvar_init_pointer(yvar_t *var, void *value) {
	if (!var)
		return (NULL);
	*var = (yvar_t){
		.refcount = -1,
		.definition = &_yvar_default_definition_g,
		.type = YVAR_POINTER,
		.pointer_value = value,
	};
	return (var);
}
/** Create a new object yvar. */
yvar_t *yvar_new_object(void *value, yvar_definition_t *definition) {
	yvar_t *var = yvar_init_object(malloc0(sizeof(yvar_t)), value, definition);
	if (var)
		var->refcount = 1;
	return (var);
}
/* Initialize a yvar structure with an object. */
yvar_t *yvar_init_object(yvar_t *var, void *value, yvar_definition_t *definition) {
	if (!var)
		return (NULL);
	*var = (yvar_t){
		.refcount = -1,
		.definition = definition,
		.type = YVAR_OBJECT,
		.pointer_value = value,
	};
	return (var);
}
/* Increments the reference counter of a yvar. */
yvar_t *yvar_retain(yvar_t *var) {
	if (!var)
		return (NULL);
	if (!var->refcount)
		return (var);
	if (var->refcount > 0)
		++var->refcount;
	else
		--var->refcount;
	return (var);
}
/* Decrements the reference counter of a yvar. */
yvar_t *yvar_release(yvar_t *var) {
	if (!var)
		return (NULL);
	if (var->refcount < 0) {
		// the yvar is static
		if (++var->refcount == 0 && var->definition && var->definition->delete_function) {
			// the embedded data is freed
			var->definition->delete_function(var);
		}
		// the yvar becomes undefined
		yvar_init_undef(var);
		return (var);
	}
	// the yvar is dynamic
	if (--var->refcount == 0) {
		// the yvar must be freed
		if (var->definition && var->definition->delete_function) {
			// the embedded data is freed
			var->definition->delete_function(var);
		}
		// the yvar is freed
		free0(var);
		return (NULL);
	}
	return (var);
}
/* Create a copy of a given yvar. */
yvar_t *yvar_clone(yvar_t *var) {
	if (!var)
		return (NULL);
	if (var->definition && var->definition->clone_function)
		return (var->definition->clone_function(var));
	return (NULL);
}

/* Free the memory allocated for a yvar. Stored value is not freed. */
void yvar_free(yvar_t *var) {
	if (!var)
		return;
	free0(var);
}
/* Recursively free a yvar. */
static ystatus_t _yvar_delete_table_item(uint64_t index, char *key, void *data,
                                         void *user_data) {
	free0(key);
	yvar_release((yvar_t*)data);
	return (YENOERR);
}
void yvar_delete(yvar_t *var) {
	yvar_release(var);
	/*
	if (!var)
		return;
	if (var->type == YVAR_BINARY)
		ybin_delete(&var->binary_value);
	if (var->type == YVAR_STRING)
		ys_free(var->string_value);
	else if (var->type == YVAR_TABLE)
		ytable_free(var->table_value);
	free0(var);
	*/
}

/* ********** TYPE ********** */
/* Tell if a yvar exists and is defined. */
bool yvar_isset(const yvar_t *var) {
	if (!var || var->type == YVAR_UNDEF)
		return (false);
	return (true);
}
/* Return the type of a yvar. */
yvar_type_t yvar_type(const yvar_t *var) {
	if (!var)
		return (YVAR_UNDEF);
	return (var->type);
}
/* Tell if a yvar is of the given type. */
bool yvar_is_a(const yvar_t *var, yvar_type_t type) {
	if (!var) {
		if (var->type == YVAR_UNDEF)
			return (true);
		return (false);
	}
	return ((var->type == type) ? true : false);
}
/* Tell if a yvar is undef. */
bool yvar_is_undef(const yvar_t *var) {
	return (yvar_is_a(var, YVAR_UNDEF));
}
/* Tell if a yvar is null. */
bool yvar_is_null(const yvar_t *var) {
	return (yvar_is_a(var, YVAR_NULL));
}
/* Tell if a yvar is a boolean value. */
bool yvar_is_bool(const yvar_t *var) {
	return (yvar_is_a(var, YVAR_BOOL));
}
/* Tell if a yvar is an integer value. */
bool yvar_is_int(const yvar_t *var) {
	return (yvar_is_a(var, YVAR_INT));
}
/* Tell if a yvar is a floating-point number value. */
bool yvar_is_float(const yvar_t *var) {
	return (yvar_is_a(var, YVAR_FLOAT));
}
/* Tell if a yvar is a constant binary value. */
bool yvar_is_const_binary(const yvar_t *var) {
	return (yvar_is_a(var, YVAR_CONST_BINARY));
}
/* Tell if a yvar is a binary value. */
bool yvar_is_binary(const yvar_t *var) {
	return (yvar_is_a(var, YVAR_BINARY));
}
/* Tell if a yvar is a constant character string value. */
bool yvar_is_const_string(const yvar_t *var) {
	return (yvar_is_a(var, YVAR_CONST_STRING));
}
/* Tell if a yvar is a character string value. */
bool yvar_is_string(const yvar_t *var) {
	return (yvar_is_a(var, YVAR_STRING));
}
/* Tell if a yvar is a table value. */
bool yvar_is_table(const yvar_t *var) {
	return (yvar_is_a(var, YVAR_TABLE));
}
/* Tell if a yvar is a table value used as an array. */
bool yvar_is_array(const yvar_t *var) {
	return (yvar_is_a(var, YVAR_TABLE) && ytable_is_array(var->table_value));
}
/* Tell if a yvar is a pointer value. */
bool yvar_is_pointer(const yvar_t *var) {
	return (yvar_is_a(var, YVAR_POINTER));
}
/* Tell if a yvar is an object value. */
bool yvar_is_object(const yvar_t *var) {
	return (yvar_is_a(var, YVAR_OBJECT));
}

/* Cast a yvar to a boolean value. */
ystatus_t yvar_cast_to_bool(yvar_t *var) {
	ystatus_t result = YENOERR;

	if (var->type == YVAR_BOOL)
		return (YENOERR);
	switch (var->type) {
		case YVAR_NULL:
			var->bool_value = false;
			break;
		case YVAR_INT:
			var->bool_value = var->int_value ? true : false;
			break;
		case YVAR_FLOAT:
			var->bool_value = var->float_value ? true : false;
			break;
		case YVAR_POINTER:
			var->bool_value = var->pointer_value ? true : false;
			break;
		case YVAR_STRING:
			{
				bool bool_value;
				if (!ys_bytesize(var->string_value) ||
				    !strcasecmp(var->string_value, "false"))
					bool_value = false;
				else if (!strcasecmp(var->string_value, "true"))
					bool_value = true;
				else if (!atoll(var->string_value))
					bool_value = false;
				else
					bool_value = true;
				ys_free(var->string_value);
				var->bool_value = bool_value;
			}
			break;
		default:
			result = YEINVAL;
	}
	if (result == YENOERR)
		var->type = YVAR_BOOL;
	return (result);
}
/* Cast a yvar to an integer value. */
ystatus_t yvar_cast_to_int(yvar_t *var) {
	ystatus_t result = YENOERR;

	if (var->type == YVAR_INT)
		return (YENOERR);
	switch (var->type) {
		case YVAR_NULL:
			var->int_value = 0;
			break;
		case YVAR_BOOL:
			var->int_value = var->bool_value ? 1 : 0;
			break;
		case YVAR_FLOAT:
			if (isnan(var->float_value) || isinf(var->float_value))
				result = YEINVAL;
			else
				var->int_value = (int64_t)var->float_value;
			break;
		case YVAR_STRING:
			{
				int64_t int_value = (int64_t)atoll(var->string_value);
				ys_free(var->string_value);
				var->int_value = int_value;
			}
			break;
		default:
			result = YEINVAL;
	}
	if (result == YENOERR)
		var->type = YVAR_INT;
	return (result);
}
/* Cast a yvar to a floating-point number value. */
ystatus_t yvar_cast_to_float(yvar_t *var) {
	ystatus_t result = YENOERR;

	if (var->type == YVAR_FLOAT)
		return (YENOERR);
	switch (var->type) {
		case YVAR_NULL:
			var->float_value = 0;
			break;
		case YVAR_BOOL:
			var->float_value = var->bool_value ? 1 : 0;
			break;
		case YVAR_INT:
			var->float_value = (double)var->int_value;
			break;
		case YVAR_STRING:
			{
				double float_value = atof(var->string_value);
				ys_free(var->string_value);
				var->float_value = float_value;
			}
			break;
		default:
			result = YEINVAL;
	}
	if (result == YENOERR)
		var->type = YVAR_FLOAT;
	return (result);
}
/* Cast a yvar to a character string value. */
ystatus_t yvar_cast_to_string(yvar_t *var) {
	ystatus_t result = YENOERR;

	if (var->type == YVAR_STRING)
		return (YENOERR);
	switch (var->type) {
		case YVAR_NULL:
			var->string_value = ys_new("");
			break;
		case YVAR_BOOL:
			var->string_value = ys_new(var->bool_value ? "1" : "");
			break;
		case YVAR_INT:
			{
				ystr_t string_value = ys_printf(NULL, "%lld",
				                                (long long)var->int_value);
				if (!string_value)
					result = YENOMEM;
				else
					var->string_value = string_value;
			}
			break;
		case YVAR_FLOAT:
			if (isnan(var->float_value) || isinf(var->float_value)) {
				result = YEINVAL;
				break;
			}
			ystr_t string_value = ys_printf(NULL, "%lf", var->float_value);
			if (!string_value)
				result = YENOMEM;
			else
				var->string_value = string_value;
			break;
		default:
			result = YEINVAL;
	}
	return (result);
}

/* Return the boolean value of a yvar. */
bool yvar_get_bool(yvar_t *var) {
	if (var->type != YVAR_BOOL)
		return (false);
	return (var->bool_value);
}
/* Return the integer value of a yvar. */
int64_t yvar_get_int(yvar_t *var) {
	if (var->type != YVAR_INT)
		return (0);
	return (var->int_value);
}
/* Return the floating-point number value of a yvar. */
double yvar_get_float(yvar_t *var) {
	if (var->type != YVAR_FLOAT)
		return (0);
	return (var->float_value);
}
/* Return the string value of a yvar. */
ystr_t yvar_get_string(yvar_t *var) {
	if (var->type != YVAR_STRING)
		return (NULL);
	return (var->string_value);
}
/* Return the table value of a yvar. */
ytable_t *yvar_get_table(yvar_t *var) {
	if (var->type != YVAR_TABLE)
		return (NULL);
	return (var->table_value);
}
/* Return the pointer value of a yvar. */
void *yvar_get_pointer(yvar_t *var) {
	if (var->type != YVAR_POINTER)
		return (NULL);
	return (var->pointer_value);
}

