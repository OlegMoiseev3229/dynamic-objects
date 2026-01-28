#pragma once

typedef struct {
	void* data;
	int offset;
	int size;
	int need_to_free;
	int initialized;
} Dyn_Arena;

typedef enum {
	Dyn_NUMBER,
	Dyn_STRING,
	Dyn_OBJECT,
} Dyn_ValueType;

struct Dyn_Object;

typedef struct {
	union {
		double n;
		char* s;
		struct Dyn_Object* o;
	};
	Dyn_ValueType t;
} Dyn_Value;

typedef struct Dyn_FieldNode {
	const char* name;
	struct Dyn_FieldNode* next;
	Dyn_Value val;
} Dyn_FieldNode;

typedef struct Dyn_Object {
	Dyn_FieldNode* fields;
} Dyn_Object;

void dyn_init(int memory_size, void* memory_buffer);
void dyn_deinit();
void dyn_free_all();
Dyn_Value dyn_number(double n);
Dyn_Value dyn_string(char* s);
Dyn_Value dyn_object_value(Dyn_Object* o);
int dyn_eq(Dyn_Value a, Dyn_Value b);
double dyn_to_number(Dyn_Value v);
char* dyn_to_string(Dyn_Value v);
void dyn_set(Dyn_Object* o, const char* name, Dyn_Value val);
Dyn_Value dyn_get(Dyn_Object* o, const char* name);
Dyn_Value dyn_get_or(Dyn_Object* o, const char* name, Dyn_Value def);
int dyn_has(Dyn_Object* o, const char* name);
int dyn_has_type(Dyn_Object* o, const char* name, Dyn_ValueType t);
#define dyn_object(...) (dyn_object_(0, __VA_ARGS__, NULL))
Dyn_Object dyn_object_(int n, ...);

#ifdef DYN_IMPLEMENTATION
#include<stdarg.h>
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
Dyn_Arena arena = {0};
void dyn_create_arena(int size) {
	arena = (Dyn_Arena) {.data=malloc(size), .size=size, .need_to_free=1};
}

void* dyn_get_memory(int bytes) {
	if (!arena.initialized) {
		fprintf(stderr, "Error: trying to allocate a dynamic object without initializing the library\n");
		return NULL;
	}
	if (arena.data == NULL) {
		fprintf(stderr, "Error: Arena in dynamic array library is NULL.\n");
		return NULL;
	}
	void* res = arena.data + arena.offset;
	arena.offset += bytes;
	if (arena.offset >= arena.size) {
		fprintf(stderr, "Ran out of memory on the arena\n");
		return NULL;
	}
	memset(res, 0, bytes);
	return res;
}

void dyn_free_all() {
	arena.offset = 0;
}

void dyn_delete_arena () {
	free(arena.data);
}

void dyn_init(int memory_size, void* memory_buffer) {
	if (arena.initialized) {
		fprintf(stderr, "Warning: dynamic array library initialized twice\n") ;
		return;
	}
	if (!memory_buffer) {
		dyn_create_arena(memory_size);
	} else {
		arena = (Dyn_Arena) {.data=memory_buffer, .size=memory_size};
	}
	arena.initialized = 1;
}

void dyn_deinit() {
	if (!arena.initialized) {
		fprintf(stderr, "Warning: trying to deinit a non initialized dynamic array library\n");
		return;
	}
	if (arena.need_to_free) {
		dyn_delete_arena();
	}
	arena.size = 0;
	arena.offset = 0;
	arena.data = NULL;
	arena.need_to_free = 0;
	arena.initialized = 0;
}

Dyn_Value dyn_number(double n) {
	return (Dyn_Value) {.t=Dyn_NUMBER, .n=n};
}

Dyn_Value dyn_string(char* s) {
	return (Dyn_Value) {.t=Dyn_STRING, .s=s};
}

Dyn_Value dyn_object_value(Dyn_Object* o) {
	return (Dyn_Value) {.t=Dyn_OBJECT, .o=o};
}

int dyn_eq(Dyn_Value a, Dyn_Value b) {
	if (a.t != b.t) {
		return 0;
	} else if (a.t == Dyn_NUMBER) {
		return a.n == b.n;
	} else if (a.t == Dyn_OBJECT) {
		return a.o == b.o;
	} else if (a.t == Dyn_STRING) {
		return !strcmp(a.s, b.s);
	} else {
		return 0;
	}

}

double dyn_to_number(Dyn_Value v) {
	if (v.t == Dyn_NUMBER) {
		return v.n;
	} else {
		return 0.;
	}
}

char* dyn_to_string(Dyn_Value v) {
	if (v.t == Dyn_STRING) {
		return v.s;
	} else if (v.t == Dyn_OBJECT) {
		return "Dyn_OBJECT";
	} else if (v.t == Dyn_NUMBER) {
		char* s = dyn_get_memory(11);
		snprintf(s, 10, "%7.2f", v.n);
		return s;
	} else {
		return "Unknown thing";
	}
}

void dyn_set(Dyn_Object* o, const char* name, Dyn_Value val) {
	Dyn_FieldNode* new_field = dyn_get_memory(sizeof(Dyn_FieldNode));
	new_field->next = o->fields;
	new_field->name = name;
	new_field->val = val;
	o->fields = new_field;
}

Dyn_Value dyn_get(Dyn_Object* o, const char* name) {
	Dyn_FieldNode* fields = o->fields;
	while (fields != NULL) {
		if (strcmp(name, fields->name) == 0) {
			return fields->val;
		}
		fields = fields->next;
	}
	return dyn_number(0.);
}

Dyn_Value dyn_get_or(Dyn_Object* o, const char* name, Dyn_Value def) {
	Dyn_FieldNode* fields = o->fields;
	while (fields != NULL) {
		if (strcmp(name, fields->name) == 0) {
			return fields->val;
		}
		fields = fields->next;
	}
	return def;
}

int dyn_has(Dyn_Object* o, const char* name) {
	Dyn_FieldNode* fields = o->fields;
	while (fields != NULL) {
		if (strcmp(name, fields->name) == 0) {
			return 1;
		}
		fields = fields->next;
	}
	return 0;
}

int dyn_has_type(Dyn_Object* o, const char* name, Dyn_ValueType t) {
	Dyn_FieldNode* fields = o->fields;
	while (fields != NULL) {
		if (strcmp(name, fields->name) == 0) {
			return fields->val.t == t;
		}
		fields = fields->next;
	}
	return 0;
}

Dyn_Object dyn_object_(int n, ...) {
	va_list args;
	Dyn_Object res = {0};
	va_start(args, n);
	char* name = va_arg(args, char*);
	if (name == NULL) {
		goto end;
	}
	Dyn_Value val = va_arg(args, Dyn_Value);
	while (name != NULL) {
		dyn_set(&res, name, val);
		name = va_arg(args, char*);
		if (name == NULL) {
			break;
		}
		val = va_arg(args, Dyn_Value);
	}
end:
	va_end(args);
	return res;
}
#endif
