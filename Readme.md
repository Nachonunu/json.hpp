# json.hpp

jsonの読み込み、編集、書き出しを行うライブラリーです。

## 使い方

c++11に対応したコンパイラが必要です。

json.hppをインクルードするだけで利用することができます。

### 読み込み

json_parse関数を利用してください。

```cpp
std::string json_text = "{\"A\" : 100, \"B\" : \"String\", \"C\" : true}";
json_node<>* json_ptr = json_parse(json_text);

delete json_ptr;
```

### 書き出し

json_node<>のprint関数を利用してください。

```cpp
json_node<>* json_ptr = new json_object<>;

std::string json_text = json_ptr->print();

delete json_ptr;
```

### 取得・編集

```cpp
json_node<>* json_ptr = new json_object<>;

//追加
json_ptr->set_object("a", new json_string<>("str"));
json_ptr->set_object("b", new json_number<>(10.12345));
json_ptr->set_object("c", new json_boolean<>(true));
json_node<>* json_d = json_ptr->set_object("d", new json_array<>);
json_node<>* json_arr_zero = json_d->set_array(0, new json_string<>("0"));
json_node<>* json_arr_one  = json_d->add_array(new json_string<>("1"));
json_node<>* json_arr_two  = json_d->add_array(new json_string<>("2"));

//削除
json_ptr->delete_object("c");
delete json_arr_one;

//検索・取得
std::string strA = json_ptr->get_object("a")->get_string();
double numB = (*json_ptr)["b"].get_number();

//編集
json_ptr->get_object("a")->set_string("change_str");
(*json_ptr)["b"].set_number(100.54321);

delete json_ptr;
```

## ライセンス

[CC0 1.0](https://creativecommons.org/publicdomain/zero/1.0/deed)
