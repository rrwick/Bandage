//Copyright 2016 Ryan Wick

//This file is part of Bandage.

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


#include "QStringList"
#include "scinot.h"
#include "math.h"

SciNot::SciNot() :
    m_coefficient(0.0), m_exponent(0)
{
    normalise();
}

SciNot::SciNot(double coefficient, int exponent) :
    m_coefficient(coefficient), m_exponent(exponent)
{
    normalise();
}


SciNot::SciNot(QString sciNotString) :
    m_coefficient(0.0), m_exponent(0)
{
    QStringList parts = sciNotString.split('e');
    if (parts.size() < 1)
        return;
    if (parts.size() < 2)
        m_coefficient = parts[0].toDouble();
    else
    {
        bool ok;
        double coefficient = parts[0].toDouble(&ok);
        if (!ok)
            return;

        double exponent = parts[1].toInt(&ok);
        if (!ok)
            return;

        m_coefficient = coefficient;
        m_exponent = exponent;
    }

    normalise();
}


SciNot::SciNot(double num) :
    m_coefficient(num), m_exponent(0)
{
    normalise();
}


void SciNot::normalise()
{
    if (m_coefficient == 0.0)
    {
        m_exponent = 0;
        return;
    }

    while (fabs(m_coefficient) >= 10.0)
    {
        m_coefficient /= 10.0;
        ++m_exponent;
    }
    while (fabs(m_coefficient) < 1.0)
    {
        m_coefficient *= 10.0;
        --m_exponent;
    }
}


bool SciNot::operator<(SciNot const &other) const
{
    if (isZero())
        return other.isPositive();

    if (isPositive())
    {
        if (other.isNegative() || other.isZero())
            return false;
        else //Both are positive
        {
            if (m_exponent < other.m_exponent) return true;
            if (m_exponent > other.m_exponent) return false;
            return m_coefficient < other.m_coefficient;
        }
    }

    else //It's negative
    {
        if (other.isPositive() || other.isZero())
            return true;
        else //Both are negative
        {
            if (m_exponent > other.m_exponent) return true;
            if (m_exponent < other.m_exponent) return false;
            return m_coefficient < other.m_coefficient;
        }
    }
}

bool SciNot::operator>(SciNot const &other) const
{
    return !(*this < other) && (*this != other);
}
bool SciNot::operator<=(SciNot const &other) const
{
    return (*this < other) || (*this == other);
}
bool SciNot::operator>=(SciNot const &other) const
{
    return (*this > other) || (*this == other);
}
bool SciNot::operator==(SciNot const &other) const
{
    return m_coefficient == other.m_coefficient && m_exponent == other.m_exponent;
}
bool SciNot::operator!=(SciNot const &other) const
{
    return !(*this == other);
}


QString SciNot::asString(bool alwaysInSciNot) const
{
    if (!alwaysInSciNot)
    {
        if (isZero())
            return "0";
        if (m_exponent < 3 && m_exponent > -3)
            return QString::number(toDouble());
    }

    return QString::number(m_coefficient) + "e" + QString::number(m_exponent);
}


double SciNot::toDouble() const
{
    return m_coefficient * pow(10.0, m_exponent);
}

bool SciNot::isValidSciNotString(QString sciNotString)
{
    QStringList parts = sciNotString.split('e');
    if (parts.size() != 2)
        return false;

    bool coefficientOk;
    parts[0].toDouble(&coefficientOk);

    bool exponentOk;
    parts[1].toInt(&exponentOk);

    return coefficientOk && exponentOk;
}

void SciNot::power(double p) {
    double newCoefficient = pow(m_coefficient, p);
    double newExponent = m_exponent * p;
    int wholePart = int(newExponent);
    double fractionalPart = newExponent - wholePart;
    m_coefficient = newCoefficient * pow(10.0, fractionalPart);
    m_exponent = wholePart;
    normalise();
}
