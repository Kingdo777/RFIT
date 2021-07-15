//
// Created by kingdo on 2021/7/15.
//

#ifndef RFIT_PING_H
#define RFIT_PING_H

#define PING(m) \
do{ \
    if(m.isping()){  \
        m.set_outputdata("PONG"); \
        return; \
    }\
}while(false);

#endif //RFIT_PING_H
