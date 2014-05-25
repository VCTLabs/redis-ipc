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
}
