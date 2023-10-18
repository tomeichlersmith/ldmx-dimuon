# extra alias for this project
alias config='cmake -B build -S .'
alias build='config && cmake --build build'
alias xsec-calc='build && ./build/dimuon-xsec-calc'
alias simulate='build && ./build/dimuon-simulate'
