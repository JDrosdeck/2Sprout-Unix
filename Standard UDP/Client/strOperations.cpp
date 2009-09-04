
#include <sstream>


using namespace std;
/*
Performs XOR operation on a string given a key
*/
string XOR(string value,string key)
{
    string retval(value);

    short unsigned int klen=key.length();
    short unsigned int vlen=value.length();
    short unsigned int k=0;
    short unsigned int v=0;
    
    for(v;v<vlen;v++)
    {
        retval[v]=value[v]^key[k];
        k=(++k < klen ? k : 0);
    }
    
    return retval;
}

/*
calcCheckSum calculates the total sum of the ASCII characters recieved in the string
*/

int calcCheckSum(string str)
{
        int a; //Number of letters
		int ASCII = 0;
        for(a = 0; a!=str.length(); ++a) /*Prints out each letter converted into a int value.*/
        {
			ASCII += int(str[a]);
		}
		return ASCII;
}


/*
trim function will trim eaither whitespace or tabs from the beginning and ends of a string
*/
void trim(string& str)
{
  string::size_type pos = str.find_last_not_of(' ');
  if(pos != string::npos) {
    str.erase(pos + 1);
    pos = str.find_first_not_of(' ');
    if(pos != string::npos) str.erase(0, pos);
  }
  else str.erase(str.begin(), str.end());

  pos = str.find_last_not_of('\t');
  if(pos != string::npos) {
    str.erase(pos + 1);
    pos = str.find_first_not_of('\t');
    if(pos != string::npos) str.erase(0, pos);
  }
  else str.erase(str.begin(), str.end());
}
