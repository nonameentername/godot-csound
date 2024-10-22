#ifndef CSOUNDGODOTDATA_H
#define CSOUNDGODOTDATA_H

class CsoundGodotData {
public:
    virtual void set_value(double key, double value) = 0;
    virtual const double get_value(double value) = 0;
};

#endif
