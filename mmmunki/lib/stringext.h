#include <string>
#include <vector>

using namespace std;

class stringext : public string {
public:
    stringext() { };
    stringext(char *s) : string(s) { };
    stringext(string s) : string(s) { };

	// split: receives a char delimiter; returns a vector of strings
    vector<stringext> split(string delimiters)
	{
		vector<stringext> fields;
		string work = data();
		string buf = "";
		for(unsigned int i = 0; i < work.length(); i++)
		{
			bool delimiter = false;
			for(unsigned int d = 0; d < delimiters.length(); d++)
				if(delimiter = (work[i] == delimiters[d]))
					break;
			if(!delimiter)
				buf += work[i];
			else if(buf.length() > 0)
			{
				fields.push_back(buf);
				buf = "";
			}
		}
		if(!buf.empty())
			fields.push_back(buf);
		return fields;
	}

	stringext upper()
	{
		string work = data();
		for(unsigned int i=0; i < work.length(); i++) work[i] = toupper(work[i]);
		return work;
	}

	stringext replace(char cFrom, char cTo)
	{
		string work = data();
		for(unsigned int i=0; i < work.length(); i++) if(work[i] == cFrom) work[i] = cTo;
		return work;
	}
	stringext cutQuotes()
	{
		string work = data();
		return work.substr(1, work.length() - 2);
	}
};
