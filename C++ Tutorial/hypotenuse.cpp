#include <iostream>
#include <cmath>

using namespace std;

int main() {
	double sideA;
	double sideB;
	double sideC;

	cout << "Welcome to the Hypotenuse calculator, lets get started" << endl;
	cout << "Enter size A: ";
	cin >> sideA;

	cout << "Enter size B: ";
	cin >> sideB;

	sideC = sqrt(pow(sideA, 2) + pow(sideB, 2));

	cout << "Calculated size of C: " << sideC;
	
	cout << "\nThanks for using this calculator!\n";

	return 0;
}

