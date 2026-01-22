#ifndef FVM_INTERFACES_ITERMINAL_H
#define FVM_INTERFACES_ITERMINAL_H

namespace fvm {
namespace interfaces {

class ITerminal {
public:
    virtual ~ITerminal() = default;
    virtual int run() = 0;
};

} // namespace interfaces
} // namespace fvm

#endif // FVM_INTERFACES_ITERMINAL_H
