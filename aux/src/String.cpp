#include "String.h"
#include <cstring>

struct String::StrRep{
    char* s;
    int ssize;
    int ref_counter;

    StrRep(int new_ssize, const char* str)
    {
        ref_counter = 1;
        ssize = new_ssize;
        s = new char[ssize + 1];
        strcpy(s, str);
    }

    ~StrRep(){delete[] s;}
    
    StrRep* get_own_copy()
    {
        if(ref_counter == 1) return this;
        ref_counter--;
        return new StrRep(ssize, s);
    }

    void assign(int new_size, const char* str)
    {
        if(ssize != new_size){
            delete[] s;
            ssize = new_size;
            s = new char[ssize + 1];
        }
        strcpy(s, str);
    }

    void add(int add_size, const char* add_str)
    {
        char* new_s = new char[ssize + add_size + 1];
        strcpy(new_s, s);
        strcpy(new_s + ssize, add_str);
        delete[] s;
        s = new_s;
        ssize += add_size;
    }


private:
    StrRep(const StrRep&);
    StrRep& operator=(const StrRep&);
};


class String::Cref{
friend class String;
    String& s;
    int i;

    Cref(String& a_s, int index) : s(a_s), i(index){}
    Cref(const Cref& ref) : s(ref.s), i(ref.i){}
    Cref();

public:
    operator char()const{s.check(i); return s.read(i);}
    void operator=(char c){s.write(i, c);}
};

String::String()
{
    representation = new StrRep(0, "");
}

String::String(const String& str)
{
    str.representation->ref_counter++;
    representation = str.representation;
}

String::~String()
{
    if(--representation->ref_counter == 0) delete[] representation;
}

String& String::operator=(const String& str)
{
    str.representation->ref_counter++;
    if(--representation->ref_counter == 0) delete[] representation;
    representation = str.representation;
    return *this;
}

String::String(const char* str)
{
    representation = new StrRep(strlen(str), str);
}

String& String::operator=(const char* str)
{
    if(representation->ref_counter == 1)
        representation->assign(strlen(str), str);
    else{
        representation->ref_counter--;
        representation = new StrRep(strlen(str), str);
    }
    return *this;
}

void String::check(int i) const{if((i < 0) || (representation->ssize <= i)) throw RangeExc();}
char String::read(int i) const{return representation->s[i];}
void String::write(int i, char c){representation = representation->get_own_copy(); representation->s[i] = c;}

String::Cref String::operator[](int i){check(i); return Cref(*this, i);}
char String::operator[](int i) const{check(i); return representation->s[i];}

int String::size() const{return representation->ssize;}

const char*const String::c_str()const{return representation->s;}

String& String::operator+=(const String& x)
{
    representation = representation->get_own_copy();
    representation->add(x.representation->ssize, x.representation->s);
    return *this;
}

String& String::operator+=(const char* x)
{
    representation = representation->get_own_copy();
    representation->add(strlen(x), x);
    return *this;
}

String operator+(const String& x, const String& y)
{
    String temp = x;
    return temp += y;
}

String operator+(const String& x, const char* y)
{
    String temp = x;
    return temp += y;
}

bool operator==(const String& x, const String& y)
{
    return (x.size() == y.size()) && (strcmp(x.c_str(), y.c_str()) == 0);
}

bool operator==(const String& x, const char* y)
{
    return (x.size() == strlen(y)) && (strcmp(x.c_str(), y) == 0);
}

bool operator!=(const String& x, const String& y)
{
    return (x.size() != y.size()) && (strcmp(x.c_str(), y.c_str()) != 0);
}

bool operator!=(const String& x, const char* y)
{
    return (x.size() != strlen(y)) && (strcmp(x.c_str(), y) != 0);
}

bool equals(const String& x, const String& y, int symbols)
{
    return strncmp(x.c_str(), y.c_str(), symbols) == 0;
}

bool equals(const String& x, const char* y, int symbols)
{
    return strncmp(x.c_str(), y, symbols) == 0;
}

