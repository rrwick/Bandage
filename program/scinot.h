//Copyright 2016 Ryan Wick

//This file is part of Bandage

//Bandage is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//Bandage is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

//You should have received a copy of the GNU General Public License
//along with Bandage.  If not, see <http://www.gnu.org/licenses/>.


#ifndef SCINOT_H
#define SCINOT_H

//This class defines a number in scientific notation.  It is needed because
//some numbers in Bandage (BLAST e-values and their products) can potentially
//exceed the limits of the C++ double type.

#include <QString>

class SciNot
{
public:
    // CREATORS
    SciNot();
    SciNot(double coefficient, int exponent);
    SciNot(QString sciNotString);
    SciNot(double num);

    // ACCESSORS
    bool operator<(SciNot const &other) const;
    bool operator>(SciNot const &other) const;
    bool operator<=(SciNot const &other) const;
    bool operator>=(SciNot const &other) const;
    bool operator==(SciNot const &other) const;
    bool operator!=(SciNot const &other) const;
    double getCoefficient() const {return m_coefficient;}
    int getExponent() const {return m_exponent;}
    QString asString(bool alwaysInSciNot) const;
    double toDouble() const;
    bool isPositive() const {return m_coefficient > 0.0;}
    bool isNegative() const {return m_coefficient < 0.0;}
    bool isZero() const {return m_coefficient == 0.0;}

    // MANIPULATORS
    void power(double p);

    // STATIC
    static bool isValidSciNotString(QString sciNotString);

private:
    double m_coefficient;
    int m_exponent;

    void normalise();
};

#endif // SCINOT_H
