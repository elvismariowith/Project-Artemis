#include "client.hpp"
#include <iostream>

int main(){

    std::string image = client::getImage();

    std::cout<<image<<'\n';
}
