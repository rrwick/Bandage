#ifndef BLASTTARGET_H
#define BLASTTARGET_H

#include <QString>

class BlastTarget
{
public:
    BlastTarget() {}
    BlastTarget(QString name, int length) : m_name(name), m_length(length) {}

    QString m_name;
    int m_length;
};

#endif // BLASTTARGET_H
