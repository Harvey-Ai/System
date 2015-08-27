#include <iostream>
using namespace std;

int main()
{
	double res = 0;

	for(int i = 1;i < 10000000;i += 4)
	{
		res += 1.0 / i - 1.0 / (i + 2);
	}
	cout << res * 4<< endl;
	return 0;
}
