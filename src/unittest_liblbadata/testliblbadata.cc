
#include "testliblbadata.h"
#include "hqrfile.h"
#include "binaryreader.h"
#include "binarywriter.h"

//-------------------------------------------------------------------------------------------
void TestLiblbadata::testBinWriter()
{
    BinaryWriter writer;
    QVERIFY(writer.size() == 0);
    writer.skip(3);
    QVERIFY(writer.size() == 3);
    writer.append((quint8)0);
    QVERIFY(writer.size() == 4);
    writer.append((qint8)0);
    QVERIFY(writer.size() == 5);
    writer.append((qint16)0);
    QVERIFY(writer.size() == 7);
    writer.append((quint16)0);
    QVERIFY(writer.size() == 9);
    writer.append(QByteArray(11,'\0'));
    QVERIFY(writer.size() == 20);
}

//-------------------------------------------------------------------------------------------
void TestLiblbadata::testBinReader()
{
    BinaryWriter writer;
    writer.append((qint8)1);
    writer.append((quint8)2);
    writer.append((qint16)3);
    writer.append((quint16)4);
    writer.skip(2);
    writer.append((quint16)5);

    BinaryReader reader(writer.buffer());
    QVERIFY(reader.readInt8()  == 1);
    QVERIFY(reader.readUint8() == 2);
    QVERIFY(reader.readInt16() == 3);
    QVERIFY(reader.readUint16()== 4);
    reader.skip(2);
    QVERIFY(reader.readUint16()== 5);
}

//-------------------------------------------------------------------------------------------
void TestLiblbadata::testHqrCompress()
{
    QVERIFY(true);
}

//-------------------------------------------------------------------------------------------
QTEST_MAIN(TestLiblbadata)

