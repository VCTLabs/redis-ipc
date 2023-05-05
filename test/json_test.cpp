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
    snook.set_field("blueness", 1.5e9);
    cout << "Display individual fields..." << endl;
    cout << snook.get_field("betook").get_field("baggins").to_string() << endl;
    cout << snook.get_field("betook").get_field("bilbo").to_string() << endl;
    cout << snook.get_field("blueness").to_string() << endl;
    cout << "Display whole thing..." << endl;
    cout << snook.dump() << endl;

    double blue = snook.get_field("blueness").to_double();
    double blue_ratio = blue / 1.5e9;
    double blue_error = 1.0 - blue_ratio;
    if (blue_error < 1e-4 && blue_error > -1e-4)
        cout << "OK double value is close enough" << endl;
    else
        cout << "ERROR double value seems fishy" << endl;

    if (snook.has_field("betook"))
        cout << "OK this field exists" << endl;
    else
        cout << "ERROR could not find field that should exist" << endl;

    if (snook.has_field("NOT-HERE"))
        cout << "ERROR found field that should NOT exist" << endl;
    else
        cout << "OK this field does not exist" << endl;

    json took_twin;
    took_twin.set_field("bilbo", "hungry");
    took_twin.set_field("baggins", 9999);

    if (took == took_twin)
        cout << "OK these objects match" << endl;
    else
        cout << "ERROR these objects were supposed to match" << endl;

    if (took != took_twin)
        cout << "ERROR these objects were supposed to match" << endl;
    else
        cout << "OK these objects match" << endl;

    if (took == snook)
        cout << "ERROR these objects were not supposed to match" << endl;
    else
        cout << "OK these objects don't match" << endl;

    if (took != snook)
        cout << "OK these objects don't match" << endl;
    else
        cout << "ERROR these objects were not supposed to match" << endl;
}
