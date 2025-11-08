
#include "utils.h"





int main() 
{
    check_fs ();
    check_packages ();
    install_missing ();
    update_packages ();

    return 0;
}