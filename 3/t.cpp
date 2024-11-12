#include <iostream>
#include <ostream>
#include "main.cpp"

int main(){
	auto factory = FunctionFactory();
	auto pol1 = factory.Create("polynomial", {5, 1});
	auto pol2 = factory.Create("polynomial", {-2, 1});
	std::cout << "pol 1: " << pol1->ToString() << std::endl;
	
	std::cout << "pol 2: " << pol2->ToString() << std::endl;
	auto pol_com = pol1 * pol2;
	std::cout << "pol_com: " << pol_com->ToString() << std::endl;
	std::cout << "pol_com(3): " << pol_com->Evaluate(3) << std::endl;
	std::cout << "pol_com_div(10): " << pol_com->Derivative(10) << std::endl;
	std::cout << FindRootByGradientDescent(pol_com, -100) << std::endl;
	std::cout << FindRootByGradientDescent(pol_com, 0) << std::endl;
}
