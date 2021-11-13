#include <iostream>
#include "json.hh"

using namespace std;

int main(int argc, char **argv)
{
    json_object *hum = json_object_new_object();
    json_object *de = json_object_new_object();

    json snook(hum), took(de);
    took.set_field("baggins", 9999);
    took.set_field("bilbo", "hungry");
    snook.set_field("betook", took);
    {
        json wooka = snook;
        cout << wooka.get_field("betook").get_field("baggins").to_string() << endl;
    }
    cout << snook.get_field("betook").get_field("baggins").to_string() << endl;
    cout << snook.get_field("betook").get_field("bilbo").to_string() << endl;

    if (snook.has_field("betook"))
        cout << "OK this field exists" << endl;
    else
        cout << "ERROR could not find field that should exist" << endl;

    if (snook.has_field("NOT-HERE"))
        cout << "ERROR found field that should NOT exist" << endl;
    else
        cout << "OK this field does not exist" << endl;
}
