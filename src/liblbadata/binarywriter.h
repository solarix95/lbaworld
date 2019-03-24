#ifndef BINARYWRITER_H
#define BINARYWRITER_H

#include <QObject>

class BinaryWriter
{
public:
    BinaryWriter();

    void clear();
    int  size() const;

    void  append(char    d);
    void  append(qint8   d);
    void  append(quint8  d);
    void  append(qint16  d);
    void  append(quint16 d);
    void  append(qint32  d);
    void  append(quint32 d);
    void  append(const QByteArray &d);

    void  skip(int bytes);

    QByteArray buffer() const;

private:
    QByteArray    mBuffer;
};

#endif // BINARYWRITER_H
