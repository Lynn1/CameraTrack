#include <iostream>  

using namespace std;

int main()
{
	double A,B,C,D;
	
	double dh1 = 6;
	double dv1 = 14;
	double len1 = 32.0156;

	double dh2 = 7;
	double dv2 = 31;
	double len2 = 67.897;

	A = (dh2-dh1)/(len2-len1);
	B = dh1-len1*A;
	C = (dv2-dv1)/(len2-len1);
	D = dv1-len1*C;

	cout<<"A: "<<A<<"    B: "<<B<<"    C: "<<C<<"    D: "<<D<<endl;
	
	cin>>A;
	return 1;

}