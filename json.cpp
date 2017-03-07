#include <stdio.h>
#include <string>
#include <vector>
#include <iostream>
#include <cstddef>
#include <fstream>
#include <streambuf>
#include <sstream>
#include <unordered_map>

struct value {
	virtual ~value() {}
	//virtual void parse(char *&f) {}
}; // Weight depends on dynamic type



struct null : value {
	int * val; 
	~null() override 
	{
		delete val;
	}
}; // Weight is constant 1



struct flag : value {
	bool val;
	~flag() override {}
}; // Weight is constant 1



struct number : value { // Ignore (,), e, n, A, B, C, D, E, F
	double  val;
	~number() override {}
}; // Weight is constant 1



struct jstring : value { // Iterate past \s
	std::string val;
	int len;

	jstring()
	{
		val = "";
		len = 0;
	}

	jstring(std::string str)
	{
		val = str;
		len = str.size();
	}
	
	~jstring() override
	{
		val.erase();
	}
}; // Weight is constant 1



struct array : value, std::vector<value*> { // Contains the weight of the number of elements in array + 1
	vector<value*> val;
	~array() override
	{
		for (value *v : val)
		{
			delete v;
		}
	}
}; // Weight of 1 + val.size()


struct object : value{ // Contains the weight of the number of mapped elements + 1
		std::unordered_map<std::string, value*> val;
		
		~object() override {}
}; // Weight of 1 + val.size()




jstring * parse_string(char *&f)
{
	value * v = new jstring;
	jstring * str = static_cast<jstring*>(v);
	str = static_cast<jstring *>(str);
	if (*f == '\"')
	{
		++f;
		str->len = 0;
		str->val = "";
		
		while (*f != '\"') // Build string value until ending quote
		{
			if (*f == '\\') // Iterate over special characters
			{
				++f;
				++f;
			}


			str->val += *f;
			str->len++;
			++f;
		}
		++f;

		return str;
	}
		else {std::cout << "Did not start string parser at a \"\n";}
}



array * parse_array(char *&f)
{
	value * v = new array;
	array * arr = static_cast<array*>(v);

	std::string content = "";
	if (*f == '[')
	{
		++f;

		while (*f != ']')
		{
			// parse value
			char * start = f;

			if (*f == '\"') // Element is a string
			{
				value * s = new jstring;
				jstring * str = static_cast<jstring*>(s);
				str = parse_string(f);
				arr->val.push_back(str); // Inserts string as value * (no data loss)
			} 
			// End string parse

		}
		++f; // Move to character after array close
		

	}	
		else {std::cout << "Did not start array parser at an [\n";};

		std::cout << "Parsed array: size " << arr->val.size() << "\n";
		return arr;
}

// Start number parsing
// Used for determining if the current position is a number value
bool isFrac(char * temp)
{
	if (*temp >= 48 && *temp <= 57)
	{
		return true;
	}
	else
	{
		return false;
	}
}
bool isNumber(char *&f)
{
	char * temp = f;

	if (*f >= 48 && *f <= 57) // Number value is an integer
	{
		return true;
	}
	else if ( *f == '.')
	{
		++temp;
		return isFrac(temp); // Number value is a fraction
	}
	else if ( *f == 'e' || *f == 'E') // Number value is exponential
	{
		++f;
		if ( *f == '+' || *f == '-')
		{
			++f;	
		}
		return false; // Instructed to ignore these values
	}
	else 
	{
		return false;				
	}
}
number * parse_number(char *&f)
{
	value * v = new number;
	number * n = static_cast<number*>(v);
	std::string temp = "";
		
	while (isNumber(f))
	{
		temp += *f;
		if (*f == '.')
		{
			++f;
			temp += *f;
		}
		++f;
	}
	n->val = std::stod(temp);

	return n;
} // End number parsing


flag * parse_flag(char *&f)
{
	value * v = new flag;
	flag * b = static_cast<flag*>(v);

	std::string temp = "";
	char * t = f;
	if (*f == 't' || *f == 'f')
	{
		temp += *f;
		for (int i = 0; i < 3; i++)
		{
			temp += *++f;
		}
		if (*t == 'f')
		{
			temp += *++f;
		}
	}
	if (temp == "true") b->val = true;
	else if (temp == "false") b->val = false;
	else std::cout << "Flag parsing error, not true or false\n";

	return b;
}

null * parse_null(char *&f)
{
	value * v = new null;
	null * n = static_cast<null*>(v);

	std::string temp = "";

	if (*f == 'n')
	{
		temp += *f;
		for (int i = 0; i < 3; i++)
		{
			temp += *++f;
		}
		n->val = nullptr;
	}
	return n;
}

value * parse(char *&f, char *&l)
{
char *temp = f; // Hold value for resetting f

std::string chunk = "";
	while (f < l) // Go through file and remove all spaces and backslashes
	{
		
		if (std::isspace(*f))
		{
			++f;
		}
		else if (!std::isspace(*f))
		{
			chunk += *f;
			++f;
		}

	}
	// End formatting
	f = temp; // Reset f
	
// Parse JSON file interating from first character to last character
// Each parse_<value> function modifies f's value
while (f != l)
{
	if (*f == '[')
	{
		parse_array(f);
	}
	else if (*f == '\"')
	{
		parse_string(f);
	}
	else if (isNumber(f))
	{
		parse_number(f);
	}
	else if (*f == 't' || *f == 'f')
	{
		parse_flag(f);
	}
	else if (*f == 'n')
	{
		parse_null(f);
	}
	
/* parse_object(char *&f) needs to be completed before this program
 * can execute properly. All parsers called above are complete and
 * have been tested for execution
 
	else if (*f == '{')
	{
		parse_object(f);
	}
*/	
}
}

std::string loadfile(std::string filename) // Used for reading in the JSON file
{
	std::string content;
	std::ifstream file(filename);
	std::stringstream stream;
	stream << file.rdbuf();
	content = stream.str();
	file.close();
	return content;
}

int main(int argc, char **argv)
{
	//std::string test = loadfile("front.json");
	std::string test = loadfile(argv[1]);
	char * f = &test[0];
	char * l = &test[0] + test.size();

	parse(f, l); // Currently causes infinite loop due to unfinished object parser
	return 0;
}
