
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

    // 0 bytes
    {
        HqrFile inHqr;
        inHqr.appendBlock(QByteArray("12345678",8));

        HqrFile outHqr;
        outHqr.fromBuffer(inHqr.toByteArray());

        QVERIFY(inHqr.count() == outHqr.count());
        QVERIFY(inHqr.block(0) == outHqr.block(0));
    }

    // exact 8 bytes
    {
        HqrFile inHqr;
        inHqr.appendBlock(QByteArray("12345678",8));

        HqrFile outHqr;
        outHqr.fromBuffer(inHqr.toByteArray());

        QVERIFY(inHqr.count() == outHqr.count());
        QVERIFY(inHqr.block(0) == outHqr.block(0));
    }


    // Test Random Data 0 byte - 20'000 bytes
    {
        QByteArray nextTestString;
        do {
            HqrFile inHqr;
            inHqr.appendBlock(nextTestString);

            HqrFile outHqr;
            outHqr.fromBuffer(inHqr.toByteArray());

            QVERIFY(inHqr.count() == outHqr.count());
            QVERIFY(inHqr.block(0) == outHqr.block(0));
            nextTestString += (char)(qrand() % 256);
        } while (nextTestString.size() < 20000);
    }

    // Test easy compressable data 0 byte - 20'000 bytes
    {
        QByteArray nextTestString;
        do {
            HqrFile inHqr;
            inHqr.appendBlock(nextTestString);

            HqrFile outHqr;
            outHqr.fromBuffer(inHqr.toByteArray());

            QVERIFY(inHqr.count() == outHqr.count());
            QVERIFY(inHqr.block(0) == outHqr.block(0));
            nextTestString += 'X';
        } while (nextTestString.size() < 20000);
    }



}

//-------------------------------------------------------------------------------------------
QTEST_MAIN(TestLiblbadata)

