#include <e32std.h>

GLDEF_C TInt E32Dll(TDllReason /*aReason*/) { // no need for thread local storage
    return(KErrNone);
}
