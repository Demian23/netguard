#ifndef MY_STRING_DEF
#define MY_STRING_DEF

class String{
struct StrRep;
class Cref;
public:
    class RangeExc{};
    String();
    String(const char*);
    String(const String&);
    String& operator=(const char*);
    String& operator=(const String&);
    ~String();

    inline void check(int i) const;
    inline char read(int i) const;
    inline void write(int i, char c);
    int size() const;

    Cref operator[](int i);    
    char operator[](int i) const;
    String& operator+=(const String& x);
    String& operator+=(const char* x);
    inline const char*const c_str()const; // use only when there is no doubt that object will 
                         // remain while this c string in use
private:
    StrRep* representation;
};

String operator+(const String& x, const String& y);
String operator+(const String& x, const char* y);
bool operator==(const String& x, const String& y);
bool operator==(const String& x, const char* y);
bool operator!=(const String& x, const String& y);
bool operator!=(const String& x, const char* y);
bool equals(const String& x, const String& y, int symbols);
bool equals(const String& x, const char* y, int symbols);



#endif // !MY_STRING_DEF
