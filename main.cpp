#include "json.hpp"

#include <string>
#include <iostream>
using namespace std;

int main() {
	json_object<> object;
	
	object.set_object("a", new json_string<>("str"));
	object.set_object("b", new json_object<>);
	object.set_object("c", new json_number<>(10.12345));
	object.set_object("d", new json_array<>);

	object["b"].set_object("test", new json_string<>("json"));
	object["b"].set_object("test2", new json_string<>("main"));

	object["d"].set_array(0, new json_string<>("Array0"));
	object["d"].set_array(1, new json_string<>("Array1"));
	object["d"].add_array(new json_string<>("Array2"));
	object["d"].add_array(new json_string<>("Array3"));
	object["d"].add_array(new json_string<>("Array4"));
	object["d"].add_array(new json_string<>("Array5"));

	delete &(object["b"]["test2"]);
	delete &(object["d"][1]);
	cout << object["d"].size() << endl;
	object["d"].delete_empty();
	cout << object.size() << endl;
	cout << object["d"][2].get_string() << endl;
	cout << object.print("", -1) << endl;

	string json = "[ 120e03, 1.234, \"\\u003cabcd\\u003e 1234\\n  5678\\t\", { \"abc\" : false, \"def\" : true, \"ghi\" : null, \"abc\" : 150 } ]";
	json_node<>* j = json_parse(json);
	if (j == nullptr) cout << "nullptr" << endl;
	else cout << j->print() << endl;
	cout << (*j)[2].get_string() << endl;
	delete j;

	return 0;
}
