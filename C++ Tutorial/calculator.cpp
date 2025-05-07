#include <iostream>
#include <cmath>

using namespace std;

int main () {
	char op;
	double num1;
	double num2;
	double result;
	int error = 0;
	
	cout << "Enter in an operation (+ , - , * , /) : ";
	cin >> op;
	
	cout << "Enter in number 1: ";
	cin >> num1;
	
	cout << "Enter in number 2: ";
	cin >> num2;

	switch(op) {
		case '+':
			result = num1 + num2;
			break;
		case '-':
			result = num1 - num2;
			break;
		case '*':
			result = num1 * num2;
			break;
		case '/':
			result = num1 / num2;
			break;
		default:
			cout << "In proper operation!" << endl;
			error = 1;
			break;
	}

	if (!error) {
		cout << "Result: " << result << endl;
	}

	return 0;
}
