// Copyright (c) 2011-2021 Vanguard Computer Technology Labs <answers@vctlabs.com>
//
// SPDX-License-Identifier: GPL-2.0-only

#ifndef __JSON_HH__
#define __JSON_HH__

#include <json-c/json.h>
#include <string>
#include <stdexcept>

class json_missing_field : public std::runtime_error
{
 public:
    explicit json_missing_field(const std::string &field_name) : std::runtime_error(field_name + " field is missing") {}
};

class json {
 public:
    // normally want to take reference on underlying json_object*,
    // except for case of initializing from an exising raw json_object*
    // such as those returned by redis_ipc -- those start out with a reference
    explicit json(bool is_array = false) {
        if (is_array) obj = json_object_new_array();
        else
            obj = json_object_new_object();
    }
    json(const json &copy) { obj = copy.obj; json_object_get(obj); }
    explicit json(const char *json_text) {
        if (json_text) obj = json_tokener_parse(json_text);
        else
            obj = json_object_new_object();
    }
    explicit json(json_object *c_obj) : obj(c_obj) {
        if (obj) json_object_get(obj);
        else
            obj = json_object_new_object();
    }
    // release reference on underlying json_object*,
    // if this was last reference it will get freed
    ~json() { json_object_put(obj); }

    json& operator=(const json &copy) { obj = copy.obj; json_object_get(obj); return *this; }

    bool operator==(const json &other) {
        bool is_same = json_object_equal(this->obj, other.obj);
        return is_same;
    }

    bool operator!=(const json &other) {
        bool is_same = json_object_equal(this->obj, other.obj);
        return !is_same;
    }

    std::string to_string() const {
        std::string value;
        if (obj) value = json_object_get_string(obj);
        // use empty string to represent empty object rather than '{ }'
        if (value == std::string("{ }")) value = std::string("");
        return value;
    }

    int to_int() const {
        int value = -1;
        if (obj) value = json_object_get_int(obj);
        return value;
    }

    bool to_bool() const {
        bool value = false;
        if (obj) value = json_object_get_int(obj);
        return value;
    }

    json_object * to_json_c_obj() { return obj; }
    const json_object * to_json_c_obj() const { return obj; }

    bool has_field(const char *field_name) {
        bool field_present = false;
        if (json_object_is_type(obj, json_type_object) &&
            json_object_object_get(obj, field_name))
        {
            field_present = true;
        }
        return field_present;
    }

    json get_field(const char *field_name) {
        if (!json_object_is_type(obj, json_type_object))
            throw std::runtime_error("Not a hash-type object!");
        json_object *field_obj = json_object_object_get(obj, field_name);
        if (field_obj == NULL) throw json_missing_field(field_name);
        json_object_get(field_obj);
        return json(field_obj);
    }

    const json get_field(const char *field_name) const {
        if (!json_object_is_type(obj, json_type_object))
            throw std::runtime_error("Not a hash-type object!");
        json_object *field_obj = json_object_object_get(obj, field_name);
        if (field_obj == NULL) throw json_missing_field(field_name);
        json_object_get(field_obj);
        return json(field_obj);
    }

    void set_field(const char *field_name, const std::string &value) {
        if (!json_object_is_type(obj, json_type_object))
            throw std::runtime_error("Not a hash-type object!");
        json_object *string_obj = json_object_new_string(value.c_str());
        json_object_object_add(obj, field_name, string_obj);
    }

    void set_field(const char *field_name, const char *value) {
        if (!json_object_is_type(obj, json_type_object))
            throw std::runtime_error("Not a hash-type object!");
        json_object *string_obj = json_object_new_string(value);
        json_object_object_add(obj, field_name, string_obj);
    }

    void set_field(const char *field_name, const int &value) {
        if (!json_object_is_type(obj, json_type_object))
            throw std::runtime_error("Not a hash-type object!");
        json_object *int_obj = json_object_new_int(value);
        json_object_object_add(obj, field_name, int_obj);
    }

    void set_field(const char *field_name, const bool &value) {
        if (!json_object_is_type(obj, json_type_object))
            throw std::runtime_error("Not a hash-type object!");
        json_object *bool_obj = json_object_new_boolean(value);
        json_object_object_add(obj, field_name, bool_obj);
    }

    // caller must make sure the 'value' object stays alive while it
    // is still used as a field
    void set_field(const char *field_name, const json &value) {
        if (!json_object_is_type(obj, json_type_object))
            throw std::runtime_error("Not a hash-type object!");
        json_object_object_add(obj, field_name, value.obj);
    }

    json get_element(int idx) {
        if (!json_object_is_type(obj, json_type_array))
            throw std::runtime_error("Not an array-type object!");
        json_object *element_obj = json_object_array_get_idx(obj, idx);
        if (element_obj == NULL) throw std::runtime_error("No such element!");
        json_object_get(element_obj);
        return json(element_obj);
    }

    const json get_element(int idx) const {
        if (!json_object_is_type(obj, json_type_array))
            throw std::runtime_error("Not an array-type object!");
        json_object *element_obj = json_object_array_get_idx(obj, idx);
        if (element_obj == NULL) throw std::runtime_error("No such element!");
        json_object_get(element_obj);
        return json(element_obj);
    }

    void set_element(int idx, const std::string &value) {
        if (!json_object_is_type(obj, json_type_array))
            throw std::runtime_error("Not an array-type object!");
        json_object *string_obj = json_object_new_string(value.c_str());
        json_object_array_put_idx(obj, idx, string_obj);
    }

    void set_element(int idx, const char *value) {
        if (!json_object_is_type(obj, json_type_array))
            throw std::runtime_error("Not an array-type object!");
        json_object *string_obj = json_object_new_string(value);
        json_object_array_put_idx(obj, idx, string_obj);
    }

    void set_element(int idx, const int &value) {
        if (!json_object_is_type(obj, json_type_array))
            throw std::runtime_error("Not an array-type object!");
        json_object *int_obj = json_object_new_int(value);
        json_object_array_put_idx(obj, idx, int_obj);
    }

    void set_element(int idx, const bool &value) {
        if (!json_object_is_type(obj, json_type_array))
            throw std::runtime_error("Not an array-type object!");
        json_object *bool_obj = json_object_new_boolean(value);
        json_object_array_put_idx(obj, idx, bool_obj);
    }

    // caller must make sure the 'value' object stays alive while it
    // is still used as a field
    void set_element(int idx, const json &value) {
        if (!json_object_is_type(obj, json_type_array))
            throw std::runtime_error("Not an array-type object!");
        json_object_array_put_idx(obj, idx, value.obj);
    }

 private:
    json_object *obj;
};

#endif  // __JSON_HH__
