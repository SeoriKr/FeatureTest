#include <iostream>

using namespace std;

// 가변 인자 템플릿의 기저 케이스 처리용 함수 (rest가 더 이상 남아있지 않을 때)
void print()
{
}

template <typename T, typename... Args>
void print(T first, Args... rest)
{
	cout << first << endl;
	print(rest...);
}


int main()
{
	print(3, 3.14f, "Hello", 'a');

	return 0;
}