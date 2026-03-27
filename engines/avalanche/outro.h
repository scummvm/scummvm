#ifndef AVALANCHE_OUTRO_H
#define AVALANCHE_OUTRO_H

namespace Avalanche {

class AvalancheEngine;

class Outro {
public:
    Outro(AvalancheEngine *vm);
    virtual ~Outro() {}
    void run();

private:
    AvalancheEngine *_vm;
};

} // End of namespace Avalanche

#endif // AVALANCHE_OUTRO_H