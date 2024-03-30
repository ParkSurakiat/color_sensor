#pragma once
#include <cstdarg>
namespace Eloquent {
    namespace ML {
        namespace Port {
            class LogisticRegression {
                public:
                    /**
                    * Predict class for features vector
                    */
                    int predict(float *x) {
                        float votes[11] = { -0.474475212527 ,1.361207053918 ,-0.604307638957 ,-0.161943332423 ,0.018422322529 ,-0.055523647663 ,-0.035916434522 ,0.001011051328 ,-0.041244255391 ,-0.01020015241 ,0.002970246117  };
                        votes[0] += dot(x,   0.230409300188  , -0.03172453902  , 0.027437461615  , 0.160240972591 );
                        votes[1] += dot(x,   0.635671223229  , -0.004473610611  , -0.353016145912  , 0.10880145996 );
                        votes[2] += dot(x,   0.601185614039  , 0.046249807056  , -0.362097432878  , 0.143513027169 );
                        votes[3] += dot(x,   -0.292434405055  , -0.154989654548  , 0.491003531313  , 0.117327099222 );
                        votes[4] += dot(x,   -0.329558824532  , -0.46827290463  , 0.775788129962  , -0.055345244548 );
                        votes[5] += dot(x,   0.432811051081  , -0.789034295852  , 0.507306414774  , -0.019749751247 );
                        votes[6] += dot(x,   -1.105151846009  , 0.546161365427  , 0.168930294895  , -0.244768264974 );
                        votes[7] += dot(x,   0.981375254781  , -0.516957164918  , -0.738324160519  , -0.116632736149 );
                        votes[8] += dot(x,   0.127311502955  , 0.585631951267  , -0.759500935853  , 0.013450043011 );
                        votes[9] += dot(x,   -0.840873698226  , 0.170482721291  , 0.492508217063  , -0.143982752161 );
                        votes[10] += dot(x,   -0.440745172452  , 0.616926324538  , -0.250035374461  , 0.037146147126 );
                        // return argmax of votes
                        uint8_t classIdx = 0;
                        float maxVotes = votes[0];

                        for (uint8_t i = 1; i < 11; i++) {
                            if (votes[i] > maxVotes) {
                                classIdx = i;
                                maxVotes = votes[i];
                            }
                        }

                        return classIdx;
                    }

                protected:
                    /**
                    * Compute dot product
                    */
                    float dot(float *x, ...) {
                        va_list w;
                        va_start(w, 4);
                        float dot = 0.0;

                        for (uint16_t i = 0; i < 4; i++) {
                            const float wi = va_arg(w, double);
                            dot += x[i] * wi;
                        }

                        return dot;
                    }
                };
            }
        }
    }
