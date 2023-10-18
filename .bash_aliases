# extra alias for this project
alias config='cmake -B build -S .'
alias build='config && cmake --build build'
alias run='build && ./build/dimuon-xsec-calc'
