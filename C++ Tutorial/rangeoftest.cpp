// using this file as a exploration / demonstration of key functionality

#include <iostream>
#include <vector>
#include <cmath>

using namespace std; // might cause conflicts as multiple names could be similar, but the entity / space is different, this assumes this specific entity


int val6 = 23; // delaring as global variable accessable by everything

int main () {
	// printing
	cout << "Print\n";

	// declaring variables
	int val1;
	double val2;
	float val3;

	// declaring variables as a const if we don't want the value to ever change
	const int val4 = 4; // will always be 4
	// val4 = 42; actually gives an error of "trying to rewrite a read only variable"
	cout << val4 << endl;

	// namespace declarations refers to the locality of the variable
	// delcarations inside of other blocks such as a loop aren't accessable outside, // but variables can be declared globally, which means its at the highest level, often outside of main 
	int val5 = 20; // declare at a higher level
		       
	// say there is a func declaration here
		val5 = 26; // this would change val5 because it was declared at a higher level
		int val7 = 25; // this is initially declared at a lower level, inside the function
		
	// val7 = 40; gives an error because its declared at a lower level, 
	

	// type casting
	typedef vector<pair<string, int>> pairlist_t; // creates a new identifier for a data type, pairlist_t can be used instead of typing all dat 
	
	typedef string bruh_t; // in replace of type def, 'using' is more efficient and better with templates
	
	using bruh_t2 = string;

	bruh_t str1 = "hello";
	cout << str1 << endl;

	bruh_t2 str2 = "bruh";
	cout << str2 << endl;
	

	// arithmetic, obviously can use math operators ( + - * /)

	int val8 = 2; val8++; // incrementing values in one line
	val8--; // decrementing
	val8*=2; // doubling 
	val8/=2; // halving
	cout << val8 << endl;

	// type conversion
	
	double  doubl1 = 3.1451;
	cout << (int) doubl1 << endl; // explicitly casts as a char, so on
	// can be used as a catch or guarentee when doing operations on dynamic values, i.e. int operations where the value could be a float, type casting the possible float value would avoid errors or unwanted behavior
	
	// reading inputs from console / command line
	
	string str4;
	cout << "wat da helly?!? : ";
	cin >> str4;

	cout << "\nBrother wat is " << str4 << endl;

	// getline(cin >> ws, str4); // does the same thing
	
	// math related functions
	
	double z = max (2, 5);
	z = min (6, 1);
	z = pow(2,4); // 2 to power of 4
	z = sqrt(25);
	z = abs(-3.2);
	z = round(141); // rounds to closest
	z = ceil(1.2); // rounds up
	z = floor(3.4); // rounds down
		       //
	
	// switch statements / case statements
	
	char grade = 'B'; // charaters must be in single quotes, strings in double (maybe both)
	switch(grade) {
		case 'A':
			cout <<"Nice" << endl;
		default:
			cout <<"Not applicable, try again";
	}
	// alternative to if statements
	
	int numgrade = 50;
	numgrade >= 60 ? cout <<"\nW work" : cout << "\nL bozo"; /// if condition, if true do this (odd) (1) : if false do this (even) (zero)
	cout << (grade >= 40 ? "\nW work" : "\nL Bozo") << endl;
	

	// logical operators, && (and) || (or) ! (not)

	// always have edge case nets

	// .length for strings

	cout << str4.length() << endl; // check if 

	// .length for strings

	cout << str4.length() << endl; // check if .empty(), .clear() to empty as string, .append('somthing') to add something to the end, .at(0) indexes the string, .insert (0, "@") inserts at a certain index, .find(' ') returns index of char, .erase(0, 3) deletes portion indexed
	/*
	str4.empty();
	str4.clear();
	str4.append("wtf");
	str4.at(0);
	str4.insert(0, "@");
	str4.find("@");
	str4.erase(0,1);
	*/	
	// more string stuff online

	// while loops to infinitly loop through until a condition is met

	// finite for loops, with index i, nested loops, so on

	// break exits a loop or level

	// pseudo random
	
	srand(time(NULL));
	int num1 = rand();
	
	for (int i = 0; i < 6; i++) {
		num1 = rand();
		cout << num1 << ' ';
	} // utiling the modulo operator % could give a random number between a range, combined with case statements random logic / events can occur.
	

	int num2 = (rand() % 100) + 1;
	int guess;
	
	do {
		cout << "Enter in a guess between (1-100): ";
		cin >> guess;
		
		if (guess > num2) {
			cout << "Guess was too big\n";
		}
		else if (guess < num2) {
			cout << "Guess was too small\n";
		}
		else {
			cout << "correct!\n";
		}
	} while(guess != num2);

	



	// functions are declared blocks of code that can be recalled, functions can be prototyped (placed before the function without the internal code)
	// then define the function after
	// function aren't aware of whats going on outside, unless passed values through parameter, or accessing global variables
	
	/*
	
	int functiontest(string str) {
		cout << str << endl;
		return 0;
	}

	int test = functiontest; this will recieve 0 from the function
	
	return 0;
	
}
