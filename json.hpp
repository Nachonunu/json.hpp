/*! json.hpp v0.1 | CC0 1.0 (https://creativecommons.org/publicdomain/zero/1.0/deed) */

#ifndef _JSON_HPP
#define _JSON_HPP

#include <algorithm>
#include <string>
#include <vector>
#include <map>
using namespace std;

enum json_type {
	None,
	Object,
	Array,
	String,
	Number,
	Boolean,
	Null
};

template <typename T> class json_object;
template <typename T> class json_array;
template <typename T> class json_string;
template <typename T> class json_number;
template <typename T> class json_boolean;
template <typename T> class json_null;

//通常文字列からJSON文字列へのエスケープ処理
template <typename T = char>
inline string json_escape_encode(const string& text) {
	string json;
	for (size_t i = 0; i < text.size(); i++) {
		switch (json[i]) {
			case '\"': json += "\\\"";  break;
			case '\\': json += "\\\\";  break;
			case '/':  json += "\\/";   break;
			case '\b': json += "\\b";   break;
			case '\f': json += "\\f";   break;
			case '\n': json += "\\n";   break;
			case '\r': json += "\\r";   break;
			case '\t': json += "\\t";   break;
			default:   json += text[i]; break;
		}
	}
	return json;
}
//JSON文字列から通常文字列へのエスケープ処理
template <typename T = char>
inline string json_escape_decode(const string& json, size_t& pos) {
	unsigned long utf32;
	string text;
	while (pos < json.size()) {
		if (json[pos] == '\"') { ++pos; return text; }
		else if (json[pos] == '\\') {
			//エスケープ処理
			++pos;
			if (pos >= json.size()) return text;
			switch (json[pos]) {
				case '\"': text += '\"'; ++pos; break;
				case '\\': text += '\\'; ++pos; break;
				case '/':  text += '/';  ++pos; break;
				case 'b':  text += '\b'; ++pos; break;
				case 'f':  text += '\f'; ++pos; break;
				case 'n':  text += '\n'; ++pos; break;
				case 'r':  text += '\r'; ++pos; break;
				case 't':  text += '\t'; ++pos; break;
				case 'u':
					//utf-8に変換
					++pos;
					if (pos+4 >= json.size()) return text; //error
					utf32 = stoul(json.substr(pos, 4), nullptr, 16); //unsinged long に変換
					if (utf32 <= 0x007F) { //1byte
						text += (char)utf32;
					} else if (utf32 >= 0x0080 && utf32 <= 0x07FF) { //2byte
						text += (char)(0xC0 | ((utf32 & 0x7C0) >> 6));
						text += (char)(0x80 |  (utf32 & 0x3F));
					} else if (utf32 >= 0x0800 && utf32 <= 0xFFFF) { //3byte
						text += (char)(0xE0 | ((utf32 & 0xF000) >> 12));
						text += (char)(0x80 | ((utf32 & 0xFC0) >> 6));
						text += (char)(0x80 |  (utf32 & 0x3F));
					} else if (utf32 >= 0x10000 && utf32 <= 0x1FFFFF) { //4byte
						text += (char)(0xF0 | ((utf32 & 0x1C0000) >> 18));
						text += (char)(0x80 | ((utf32 & 0x3F000) >> 12));
						text += (char)(0x80 | ((utf32 & 0xFC0) >> 6));
						text += (char)(0x80 |  (utf32 & 0x3F));
					} else return text; //error 未定義
					pos += 4;
					break;
				default: return text;
			}
		} else if (json[pos] >= ' ' || json[pos] < '\a') { text += json[pos]; ++pos; }
		else return text;
	}
	return text;
}

template <typename T = char>
class json_node {
protected:
	json_node<T>* n_parent;
	json_node<T>** n_parent_pos;

	void set_parent(json_node<T>* n, json_node<T>* p, json_node<T>** pos) {
		if (n->n_parent != nullptr) *n->n_parent_pos = nullptr; //親ノードとの連結を解除
		n->n_parent = p;
		n->n_parent_pos = pos;
	}
	void reset_parent() { n_parent = nullptr; n_parent_pos = nullptr; }
public:
	json_node() {}
	virtual ~json_node() {}
	virtual json_type type() { return None; }
	json_node<T>* parent() { return n_parent; }
	virtual string print(const string& indentstr = "\t", const int indent = 0) { return ""; }
	virtual json_node<T>* get_object(const string& key) { return nullptr; }
	virtual json_node<T>* get_array(const size_t num) { return nullptr; }
	virtual string get_string() { return ""; }
	virtual double get_number() { return 0; }
	virtual bool get_bool() { return false; }
	virtual json_node<T>* set_object(const string& key, json_node<T>* n) { return nullptr; }
	virtual json_node<T>* set_array(const size_t num, json_node<T>* n) { return nullptr; }
	virtual json_node<T>* add_array(json_node<T>* n) { return nullptr; }
	virtual void set_string(const string& str) {}
	virtual void set_number(const double num) {}
	virtual void set_bool(const bool b) {}
	virtual void delete_object(const string& key) {}
	virtual void delete_array(const size_t num, const int eraseflag = 0) {}
	virtual void delete_all() {}
	virtual void delete_empty() {}
	virtual void resize(const size_t s) {}
	virtual size_t size() { return 0; }

	virtual json_node<T>& operator[](const string& str) { return *this; }
	virtual json_node<T>& operator[](const size_t num) { return *this; }
};

template <typename T = char>
class json_object : public json_node<T> {
	using json_node<T>::n_parent;
	using json_node<T>::n_parent_pos;
	using json_node<T>::set_parent;
	using json_node<T>::reset_parent;

	map<string, json_node<T>**> nodelist;
	friend class json_array<T>;

	json_node<T>** find(const string& key) {
		//ノードを検索
		if (!nodelist.empty()) { //リストが存在
			auto it = nodelist.find(key); //検索
			if (it != nodelist.end()) return it->second; //mapに存在
		}
		return nullptr;
	}
public:
	json_object() { reset_parent(); }
	~json_object() {
		delete_all();
		if (n_parent != nullptr) *n_parent_pos = nullptr; //親ノードとの連結を解除
	}
	string print(const string& indentstr = "\t", const int indent = 0) {
		//JSONテキスト出力
		string out, indenttext;
		int last;
		out += "{";
		if (indent >= 0) {
			for (int i = 0; i < indent+1; i++) indenttext += indentstr;
			out += "\n";
			last = 2;
			for (auto it : nodelist)
				if (*it.second != nullptr) out += indenttext + "\"" + json_escape_encode(it.first) + "\" : " + (*it.second)->print(indentstr, indent+1) + ",\n";
		} else {
			last = 1;
			for (auto it : nodelist)
				if (*it.second != nullptr) out += "\"" + json_escape_encode(it.first) + "\":" + (*it.second)->print(indentstr, -1) + ",";
		}
		out.erase(out.end() - last);
		out += indenttext.substr(0, indent * indentstr.size()) + "}";
		return out;
	}
	json_type type() { return Object; }
	json_node<T>* get_object(const string& key) {
		//指定されたノードを取得
		json_node<T>** f_node = find(key);
		if (f_node != nullptr) { //mapに存在
			if (*f_node != nullptr) return *f_node; //中身が存在
			else {
				//中身が存在しないのでmapから削除
				delete f_node;
				nodelist.erase(key);
			}
		}
		return nullptr;
	}
	json_node<T>* set_object(const string& key, json_node<T>* n) {
		//指定されたノードを追加または変更(既存のノードを代入)
		json_node<T>** f_node = find(key); //ノードを検索
		if (f_node != nullptr) { //mapに存在
			if (*f_node != nullptr) delete *f_node; //中身が存在する場合は削除する
		} else nodelist[key] = new json_node<T>*; //リスト作成
		set_parent(n, this, nodelist[key]);
		*nodelist[key] = n;
		return n;
	}
	void delete_object(const string& key) {
		//指定されたノードを削除
		json_node<T>** f_node = find(key); //ノードを検索
		if (f_node != nullptr) {
			if (*f_node != nullptr) delete *f_node;
			delete f_node;
			nodelist.erase(key);
		}
	}
	void delete_all() {
		//すべてのノードを削除
		if (nodelist.empty()) return;
		for (auto it : nodelist) {
			if (*it.second != nullptr) delete *it.second;
			delete it.second;
		}
		nodelist.clear();
	}
	void delete_empty() {
		//空のコンテナをすべて削除
		if (nodelist.empty()) return;
		for (auto it : nodelist)
			if (*it.second == nullptr) { delete it.second; nodelist.erase(it.first); }
	}
	size_t size() { return nodelist.size(); }

	json_node<T>& operator[](const size_t num) { return *get_object(to_string(num)); }
	json_node<T>& operator[](const string& str) { return *get_object(str); }
};

template <typename T = char>
class json_array : public json_node<T> {
	using json_node<T>::n_parent;
	using json_node<T>::n_parent_pos;
	using json_node<T>::set_parent;
	using json_node<T>::reset_parent;

	vector<json_node<T>**> nodelist;
	friend class json_object<T>;
public:
	json_array() { reset_parent(); }
	json_array(const size_t num) { reset_parent(); resize(num); }
	~json_array() {
		delete_all();
		if (n_parent != nullptr) *n_parent_pos = nullptr; //親ノードとの連結を解除
	}
	string print(const string& indentstr = "\t", const int indent = 0) {
		//指定されたノードを取得
		string out, indenttext;
		int last;
		out += "[";
		if (indent >= 0) {
			for (int i = 0; i < indent+1; i++) indenttext += indentstr;
			out += "\n";
			last = 2;
		} else last = 1;
		if (indent >= 0) {
			for (json_node<T>** n : nodelist)
				if (*n != nullptr) out += indenttext + (*n)->print(indentstr, indent+1) + ",\n";
		} else {
			for (json_node<T>** n : nodelist)
				if (*n != nullptr) out += (*n)->print(indentstr, -1) + ",";
		}
		out.erase(out.end() - last);
		out += indenttext.substr(0, indent * indentstr.size()) + "]";
		return out;
	}
	json_type type() { return Array; }
	json_node<T>* get_array(const size_t num) {
		//指定されたノードを取得
		if (num >= nodelist.size()) return nullptr; //リスト存在チェック
		if (nodelist[num] == nullptr) return nullptr;
		return *nodelist[num];
	}
	json_node<T>* set_array(const size_t num, json_node<T>* n) {
		//指定されたノードを変更(既存のノードを代入)
		if (num >= nodelist.size()) resize(num+1);
		if (nodelist[num] != nullptr) { //vectorに存在
			if (*nodelist[num] != nullptr) delete *nodelist[num]; //中身が存在する場合は削除
		} else nodelist[num] = new json_node<T>*; //リスト作成
		set_parent(n, this, nodelist[num]);
		*nodelist[num] = n;
		return n;
	}
	json_node<T>* add_array(json_node<T>* n) {
		//指定されたノードを追加(既存のノードを代入)
		size_t pos = nodelist.size();
		nodelist.push_back(new json_node<T>*); //リスト追加
		set_parent(n, this, nodelist[pos]);
		*nodelist[pos] = n;
		return n;
	}
	void delete_array(const size_t num, const int eraseflag = 0) {
		//指定されたノードを削除
		if (num < nodelist.size()) return;
		if (*nodelist[num] != nullptr) delete *nodelist[num];
		*nodelist[num] = nullptr;
		if (eraseflag == 1) { delete nodelist[num]; nodelist.erase(nodelist.begin() + num); }
	}
	void delete_all() {
		//すべてのノードを削除
		for (json_node<T>** n : nodelist) {
			if (*n != nullptr) delete *n;
			delete n;
		}
		nodelist.clear();
	}
	void delete_empty() {
		//空のコンテナをすべて削除
		auto emptylist = remove_if(nodelist.begin(), nodelist.end(), [](json_node<T>** n)->bool {
			if (*n == nullptr) {
				if (n != nullptr) delete n;
				return true;
			}
			return false;
		});
		nodelist.erase(emptylist, nodelist.end());
	}
	void resize(const size_t s) {
		//リストのリサイズ
		if (s < nodelist.size()) {
			//減少分の中身を削除
			for (size_t i = nodelist.size()-1; i >= s; i--) {
				if (*nodelist[i] != nullptr) delete *nodelist[i];
				delete nodelist[i];
			}
			nodelist.resize(s);
		} else if (s > nodelist.size()) {
			size_t bsize = nodelist.size();
			nodelist.resize(s);
			//追加分を初期化
			for (size_t i = bsize; i < s; i++) {
				nodelist[i] = new json_node<T>*;
				*nodelist[i] = nullptr;
			}
		}
	}
	size_t size() { return nodelist.size(); }

	json_node<T>& operator[](const size_t num) { return *get_array(num); }
};

template <typename T = char>
class json_string : public json_node<T> {
	using json_node<T>::n_parent;
	using json_node<T>::n_parent_pos;
	using json_node<T>::reset_parent;

	string v;
	friend class json_object<T>;
	friend class json_array<T>;
public:
	json_string() { reset_parent(); }
	json_string(const string& arg) { reset_parent(); v = arg; }
	~json_string() { if (n_parent != nullptr) *n_parent_pos = nullptr; } //親ノードとの連結を解除
	string print(const string& indentstr = "\t", const int indent = 0) { return "\"" + json_escape_encode(v) + "\""; }
	json_type type() { return String; }
	string get_string() { return v; }
	double get_number() { return stod(v); }
	bool get_bool() { return v == "ture" ? true : false; }
	void set_string(const string& str) { v = str; }
	void set_number(const double num) { v = to_string(num); }
	void set_bool(const bool b) { v = b ? "true" : "false"; }
};

template <typename T = char>
class json_number : public json_node<T> {
	using json_node<T>::n_parent;
	using json_node<T>::n_parent_pos;
	using json_node<T>::reset_parent;

	double v;
	friend class json_object<T>;
	friend class json_array<T>;
public:
	json_number() { reset_parent(); v = 0; }
	json_number(double arg) { reset_parent(); v = arg; }
	~json_number() { if (n_parent != nullptr) *n_parent_pos = nullptr; }
	string print(const string& indentstr = "\t", const int indent = 0) { return to_string(v); }
	json_type type() { return Number; }
	string get_string() { return to_string(v); }
	double get_number() { return v; }
	bool get_bool() { return v != 0 ? true : false; }
	void set_string(const string& str) { v = stod(str); }
	void set_number(const double num) { v = num; }
	void set_bool(const bool b) { v = b ? 1 : 0; }
};

template <typename T = char>
class json_boolean : public json_node<T> {
	using json_node<T>::n_parent;
	using json_node<T>::n_parent_pos;
	using json_node<T>::reset_parent;

	bool v;
	friend class json_object<T>;
	friend class json_array<T>;
public:
	json_boolean() { reset_parent(); v = false; }
	json_boolean(bool arg) { reset_parent(); v = arg; }
	~json_boolean() { if (n_parent != nullptr) *n_parent_pos = nullptr; } //親ノードとの連結を解除
	string print(const string& indentstr = "\t", const int indent = 0) { return v ? "true" : "false"; }
	json_type type() { return Boolean; }
	string get_string() { return v ? "true" : "false"; }
	double get_number() { return v ? 1 : 0; }
	bool get_bool() { return v; }
	void set_string(const string& str) { v = (str == "true") ? true : false; }
	void set_number(const double num) { v = (num != 0) ? true : false; }
	void set_bool(const bool b) { v = b; }
};

template <typename T = char>
class json_null : public json_node<T> {
	using json_node<T>::n_parent;
	using json_node<T>::n_parent_pos;
	using json_node<T>::reset_parent;

	friend class json_object<T>;
	friend class json_array<T>;
public:
	json_null() { reset_parent(); }
	~json_null() { if (n_parent != nullptr) *n_parent_pos = nullptr; } //親ノードとの連結を解除
	string print(const string& indentstr = "\t", const int indent = 0) { return "null"; }
	json_type type() { return Null; }
};

template <typename T = char>
json_node<T>* json_parse_type(const string& json, size_t& pos) {
	json_node<T>* node = nullptr;
	size_t now = 0;
	int cnt, flag;
	string str;
	while (pos < json.size()) {
		switch (json[pos]) {
			case ' ': case '\n': case '\t': //whitespace
				++pos;
				break;
			case '{': //object
				node = new json_object<T>;
				++pos; flag = 0; //-1=終了, 0=キーなし, 1=キーあり, 2=ノードあり
				while (pos < json.size()) {
					switch (json[pos]) {
						case ' ': case '\n': case '\t':
							++pos;
							break;
						case '\"':
							//キー作成
							if (flag == 0) {
								++pos; flag = 1;
								str = json_escape_decode(json, pos);
							} else { delete node; return nullptr; }
							break;
						case ':':
							//ノード作成
							if (flag == 1) {
								++pos;
								if (node->set_object(str, json_parse_type<T>(json, pos)) == nullptr) { delete node; return nullptr; } //ノード作成失敗
								flag = 2;
							} else { delete node; return nullptr; }
							break;
						case ',':
							//ノード確定処理
							if (flag == 2) { ++pos; flag = 0; }
							else { delete node; return nullptr; }
							break;
						case '}':
							//終了
							if (flag == 2 || node->size() == 0) { ++pos; return node; }
							delete node;
							return nullptr;
						default:
							delete node;
							return nullptr;
					}
				}
				delete node;
				return nullptr;
			case '[': //array
				node = new json_array<T>;
				++pos; flag = 0; //-1=終了, 0=ノードなし, 1=ノードあり
				while (pos < json.size()) {
					switch (json[pos]) {
						case ' ': case '\n': case '\t':
							++pos;
							break;
						case ',':
							//ノード確定処理
							if (flag == 1) { ++pos; flag = 0; }
							else { delete node; return nullptr; }
							break;
						case ']':
							//終了
							if (flag == 1 || node->size() == 0) { ++pos; return node; }
							delete node;
							return nullptr;
						default:
							if (flag == 0) {
								//ノード作成
								if (node->add_array(json_parse_type<T>(json, pos)) == nullptr) { delete node; return nullptr; } //ノード作成失敗
								flag = 1;
							} else { delete node; return nullptr; }
							break;
					}
				}
				delete node;
				return nullptr;
			case '\"': //string
				++pos;
				return new json_string<T>(json_escape_decode(json, pos));
			case '-':
				//number - (次の文字が数字以外の場合はエラー)
				if (pos+1 < json.size() && (json[pos+1] < 0x30 || json[pos+1] > 0x39)) return nullptr;
				now = 1;
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
				//number
				flag = 0; cnt = 0; now += pos; //flag(-1=数字終了, 0=整数, 1=小数点, 2=指数) ,cnt(数字カウント)
				while (now < json.size()) {
					switch (json[now]) {
						case ' ': case '\t': case '\n':
							++now;
						case ',':
							//終了
							node = new json_number<T>( stod( json.substr(pos, now-pos) ) );
							pos = now;
							return node;
						case 'e':
						case 'E': // e, E, e+, e-, E+, E-
							//指数モードに変更
							if ((flag == 0 || flag == 1) && cnt > 0 && now+1 < json.size() && (json[now+1] == '-' || json[now+1] == '+' || (json[now+1] >= 0x30 && json[now+1] <= 0x39))) {
								flag = 2; cnt = 0; now += 2;
							} else return nullptr;
							break;
						case '.':
							//小数点モードに変更
							if (flag != 0 || cnt == 0) return nullptr; //整数モード以外の場合はエラー
							flag = 1; cnt = 0; ++now;
							break;
						case '0':
							//整数モードの最初の文字が0 + 次の文字が数字の場合はエラー
							if (cnt == 0 && flag == 0 && now+1 < json.size() && json[now+1] >= 0x30 && json[now+1] <= 0x39) return nullptr;
						case '1': case '2': case '3': case '4': case '5':
						case '6': case '7': case '8': case '9':
							++cnt; ++now;
							break;
						default: return nullptr;
					}
				}
				return nullptr;
			case 't': //boolean true
				if (json.compare(pos+1, 3, "rue") == 0) {
					pos += 4;
					return new json_boolean<T>(true);
				}
				return nullptr;
			case 'f': //boolean false
				if (json.compare(pos+1, 4, "alse") == 0) {
					pos += 5;
					return new json_boolean<T>(false);
				}
				return nullptr;
			case 'n': //null
				if (json.compare(pos+1, 3, "ull") == 0) {
					pos += 4;
					return new json_null<T>;
				}
				return nullptr;
			default: return nullptr;
		}
	}
	return nullptr;
}

template <typename T = char>
json_node<T>* json_parse(const string& json) {
	//utf8 BOM判定を省略
	const unsigned char* bom = (unsigned char*)json.c_str();
	size_t pos = (json.size() >= 3 && bom[0] == 0xEF && bom[1] == 0xBB && bom[2] == 0xBF) ? 3 : 0;
	return json_parse_type<T>(json, pos);
}
#endif //_JSON_HPP
