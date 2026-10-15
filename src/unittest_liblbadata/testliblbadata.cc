
#include "testliblbadata.h"
#include "hqrfile.h"
#include "binaryreader.h"
#include "binarywriter.h"
#include "lbaanimation.h"
#include "lbacharacter.h"
#include "lbapalette.h"
#include "lbaress.h"
#include <QDir>
#include <QFile>
#include <QRandomGenerator>
#include <QTemporaryDir>

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

    BinaryWriter endianWriter;
    endianWriter.append(quint16(0x1234));
    endianWriter.append(quint32(0x12345678));
    QCOMPARE(endianWriter.buffer().toHex(), QByteArray("341278563412"));
}

//-------------------------------------------------------------------------------------------
void TestLiblbadata::testBinReaderErrors()
{
    BinaryReader reader(QByteArray::fromHex("3412"));
    QCOMPARE(reader.readUint16(), quint16(0x1234));
    QVERIFY(reader.atEnd());
    QCOMPARE(reader.readUint8(), quint8(0));
    QVERIFY(reader.error());

    reader.setBuffer(QByteArray::fromHex("78563412"));
    QVERIFY(!reader.error());
    QCOMPARE(reader.getUint32(0), quint32(0x12345678));
    QCOMPARE(reader.getUint32(1), quint32(0));
    QVERIFY(reader.error());
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
void TestLiblbadata::testHqrFile()
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
        QRandomGenerator random(0x12345678);
        do {
            HqrFile inHqr;
            inHqr.appendBlock(nextTestString);

            HqrFile outHqr;
            outHqr.fromBuffer(inHqr.toByteArray());

            QVERIFY(inHqr.count() == outHqr.count());
            QVERIFY(inHqr.block(0) == outHqr.block(0));
            nextTestString += char(random.generate() & 0xff);
        } while (nextTestString.size() < 500);
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
        } while (nextTestString.size() < 2000);
    }

    // Big File with maximum compressable data
    {
        QByteArray testString(5000000,'X');

        HqrFile inHqr;
        inHqr.appendBlock(testString);

        HqrFile outHqr;
        QByteArray compressedHqrBuffer = inHqr.toByteArray(2);
        // qDebug() << "Compressed to" << 100*(compressedHqrBuffer.size()/(double)testString.size()) << "%";
        outHqr.fromBuffer(compressedHqrBuffer);

        QVERIFY(inHqr.count() == outHqr.count());
        QVERIFY(inHqr.block(0) == outHqr.block(0));
    }

    // Big File with (compressable) random data
    {
        QByteArray testString;
        QRandomGenerator random(0x87654321);
        do {
           testString += char(random.generate() % 2);
        } while (testString.size() < 2000000);  // 5Megs

        HqrFile inHqr;
        inHqr.appendBlock(testString);

        HqrFile outHqr;
        QByteArray compressedHqrBuffer = inHqr.toByteArray(2);
        // qDebug() << "Compressed to" << 100*(compressedHqrBuffer.size()/(double)testString.size()) << "%";
        outHqr.fromBuffer(compressedHqrBuffer);

        QVERIFY(inHqr.count() == outHqr.count());
        QVERIFY(inHqr.block(0) == outHqr.block(0));
    }
}

//-------------------------------------------------------------------------------------------
void TestLiblbadata::testHqrLazyFileAccess()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());

    HqrFile source;
    source.appendBlock("first block");
    source.appendBlock(QByteArray());
    source.appendBlock(QByteArray(2048, 'X'));

    const QString filename = directory.filePath("BODY.HQR");
    QFile output(filename);
    QVERIFY(output.open(QIODevice::WriteOnly));
    const QByteArray serialized = source.toByteArray(2);
    BinaryReader serializedReader(serialized);
    QCOMPARE(serializedReader.getUint32(0), quint32(16)); // 3 entries + EOF offset
    QCOMPARE(serializedReader.getUint32(12), quint32(serialized.size()));
    QCOMPARE(output.write(serialized), qint64(serialized.size()));
    output.close();

    HqrFile archive;
    QVERIFY2(archive.fromFile(filename), qPrintable(archive.errorString()));
    QVERIFY(archive.isOpen());
    QVERIFY(archive.isValid());
    QCOMPARE(archive.count(), 3);
    QCOMPARE(archive.cachedBlockCount(), 0);
    QCOMPARE(archive.entryInfo(0).decompressedSize, quint32(11));

    bool ok = false;
    QCOMPARE(archive.data(0, &ok), QByteArray("first block"));
    QVERIFY(ok);
    QCOMPARE(archive.cachedBlockCount(), 1);
    QCOMPARE(archive.data(2, &ok), QByteArray(2048, 'X'));
    QVERIFY(ok);
    QCOMPARE(archive.cachedBlockCount(), 2);

    archive.clearCache();
    QCOMPARE(archive.cachedBlockCount(), 0);
    QCOMPARE(archive.data(-1, &ok), QByteArray());
    QVERIFY(!ok);
}

//-------------------------------------------------------------------------------------------
void TestLiblbadata::testHqrValidation()
{
    // Backwards compatibility with archives produced by older lbaworld
    // versions, which did not write the final EOF offset.
    BinaryWriter legacy;
    legacy.append(quint32(4));
    legacy.append(quint32(3));
    legacy.append(quint32(3));
    legacy.append(quint16(0));
    legacy.append(QByteArray("old"));
    HqrFile legacyArchive;
    QVERIFY(legacyArchive.fromBuffer(legacy.buffer()));
    QCOMPARE(legacyArchive.count(), 1);
    QCOMPARE(legacyArchive.data(0), QByteArray("old"));

    BinaryWriter badHeader;
    badHeader.append(quint32(3));
    HqrFile invalidHeader;
    QVERIFY(!invalidHeader.fromBuffer(badHeader.buffer()));
    QVERIFY(!invalidHeader.errorString().isEmpty());

    BinaryWriter truncated;
    truncated.append(quint32(4));
    truncated.append(quint32(10));
    truncated.append(quint32(100));
    truncated.append(quint16(1));
    HqrFile truncatedArchive;
    QVERIFY(truncatedArchive.fromBuffer(truncated.buffer()));
    QVERIFY(!truncatedArchive.isValid());
    QVERIFY(!truncatedArchive.validationErrors().isEmpty());

    BinaryWriter badBackReference;
    badBackReference.append(quint32(4));
    badBackReference.append(quint32(3));
    badBackReference.append(quint32(3));
    badBackReference.append(quint16(1));
    badBackReference.append(quint8(0));
    badBackReference.append(quint16(0));

    HqrFile invalidData;
    QVERIFY(invalidData.fromBuffer(badBackReference.buffer()));
    QVERIFY(invalidData.isValid()); // The index is valid before payload verification.
    QStringList errors;
    QVERIFY(!invalidData.validate(&errors, true));
    QVERIFY(!invalidData.isValid());
    QVERIFY(!errors.isEmpty());
    QVERIFY(errors.first().contains("back-reference"));
}

//-------------------------------------------------------------------------------------------
void TestLiblbadata::testLbaRess()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    QVERIFY(QDir(directory.path()).mkpath("nested"));

    HqrFile bodySource;
    bodySource.appendBlock("runtime model");
    QFile bodyFile(directory.filePath("nested/BODY.HQR"));
    QVERIFY(bodyFile.open(QIODevice::WriteOnly));
    const QByteArray bodyData = bodySource.toByteArray(2);
    QCOMPARE(bodyFile.write(bodyData), qint64(bodyData.size()));
    bodyFile.close();

    QFile trackFile(directory.filePath("track01.ogg"));
    QVERIFY(trackFile.open(QIODevice::WriteOnly));
    trackFile.write("audio");
    trackFile.close();

    LbaRess resources;
    QVERIFY(resources.loadFromDirectory(LbaRess::LBA1, directory.path()));
    QVERIFY(resources.hasArchive(LbaRess::LBA1, LbaRess::Body));
    QCOMPARE(resources.count(LbaRess::LBA1, LbaRess::Body), 1);
    QCOMPARE(resources.archive(LbaRess::LBA1, LbaRess::Body)->cachedBlockCount(), 0);

    bool ok = false;
    QCOMPARE(resources.data(LbaRess::LBA1, LbaRess::Body, 0, &ok), QByteArray("runtime model"));
    QVERIFY(ok);
    QCOMPARE(resources.archive(LbaRess::LBA1, LbaRess::Body)->cachedBlockCount(), 1);
    QVERIFY(resources.tracks().contains("track01"));
    QCOMPARE(resources.validationErrors(LbaRess::LBA1), QStringList());

    LbaRess::Source source;
    LbaRess::Content content;
    QString ident;
    QVERIFY(LbaRess::fromUrl("lba1://BODY.HQR/0", source, content, ident));
    QCOMPARE(source, LbaRess::LBA1);
    QCOMPARE(content, LbaRess::Body);
    QCOMPARE(ident, QString("0"));
    QCOMPARE(LbaRess::toUrl(source, content, ident.toInt()), QString("lba1://body/0"));

    QVERIFY(LbaRess::fromUrl("2/spl/42", source, content, ident));
    QCOMPARE(source, LbaRess::LBA2);
    QCOMPARE(content, LbaRess::Samples);
    QCOMPARE(ident, QString("42"));
}

//-------------------------------------------------------------------------------------------
void TestLiblbadata::testLbaAnimation()
{
    BinaryWriter writer;
    writer.append(quint16(3)); // keyframes
    writer.append(quint16(2)); // bones
    writer.append(quint16(1)); // loop start
    writer.append(quint16(0)); // reserved

    const auto appendFrame = [](BinaryWriter &target, quint16 duration, qint16 value) {
        target.append(duration);
        target.append(qint16(0x400));
        target.append(qint16(-0x400));
        target.append(qint16(0x200));

        target.append(quint16(0)); // rotation
        target.append(value);
        target.append(qint16(0));
        target.append(qint16(-value));

        target.append(quint16(1)); // translation
        target.append(qint16(100));
        target.append(qint16(200));
        target.append(qint16(-300));
    };

    appendFrame(writer, 5, 0x100);
    appendFrame(writer, 7, 0x080);
    appendFrame(writer, 11, 0x040);

    LbaAnimation animation;
    QVERIFY2(animation.fromBuffer(writer.buffer()), qPrintable(animation.errorString()));
    QVERIFY(animation.isValid());
    QCOMPARE(animation.keyFrameCount(), 3);
    QCOMPARE(animation.boneCount(), 2);
    QCOMPARE(animation.loopStartIndex(), 1);
    QCOMPARE(animation.durationTicks(), 23);
    QCOMPARE(animation.playbackLoopStartTicks(), 23);
    QCOMPARE(animation.playbackDurationTicks(), 41);
    QCOMPARE(animation.playbackTimeline().count(), 6);
    QCOMPARE(animation.playbackTimeline().at(3).time, 23.0f);
    QCOMPARE(animation.playbackTimeline().last().time, 41.0f);
    QCOMPARE(animation.keyFrames().count(), 3);
    QVERIFY(animation.keyFrame(1));
    QCOMPARE(animation.keyFrame(1)->keyFrameLength, 7);
    QCOMPARE(animation.keyFrame(0)->velocityX, 1.0f);
    QCOMPARE(animation.keyFrame(0)->velocityY, -1.0f);
    QCOMPARE(animation.keyFrame(0)->velocityZ, 0.5f);
    QCOMPARE(animation.bones(0).at(0).boneType, 0);
    QCOMPARE(animation.bones(0).at(0).rawX, qint16(0x100));
    QCOMPARE(animation.bones(0).at(0).rotateX, 90.0f);
    QCOMPARE(animation.bones(0).at(1).boneType, 1);
    QCOMPARE(animation.bones(0).at(1).rotateZ, -300.0f);
    QCOMPARE(animation.keyFrame(0)->stepX, qint16(0x400));

    const LbaAnimation::RootMotion halfFirstFrame = animation.rootMotionAt(2.5f);
    QCOMPARE(halfFirstFrame.translation, QVector3D(0.64f, -0.64f, 0.32f));
    QCOMPARE(halfFirstFrame.rotationY, 0.0f);
    const LbaAnimation::RootMotion originalEnd = animation.rootMotionAt(23.0f);
    QCOMPARE(originalEnd.translation, QVector3D(3.84f, -3.84f, 1.92f));

    QVERIFY(!animation.keyFrame(-1));
    QVERIFY(!animation.keyFrame(3));
    QVERIFY(animation.bones(-1).isEmpty());
    QVERIFY(animation.bones(3).isEmpty());

    QVERIFY(!animation.fromBuffer(writer.buffer().left(writer.size()-1)));
    QVERIFY(!animation.isValid());
    QCOMPARE(animation.keyFrameCount(), 0);
    QVERIFY(!animation.errorString().isEmpty());
}

//-------------------------------------------------------------------------------------------
void TestLiblbadata::testLbaCharacter()
{
    BinaryWriter writer;

    writer.append(quint8(LbaCharacter::OPBODY));
    writer.append(quint8(0));       // logical body
    writer.append(quint8(17));      // distance from size byte to next block
    writer.append(qint16(7));       // BODY.HQR index
    writer.append(quint8(1));       // collision box present
    writer.append(quint8(14));
    writer.append(qint16(-250));
    writer.append(qint16(0));
    writer.append(qint16(-300));
    writer.append(qint16(250));
    writer.append(qint16(1200));
    writer.append(qint16(300));

    writer.append(quint8(LbaCharacter::OPANIM));
    writer.append(quint8(0));       // idle
    writer.append(quint8(4));
    writer.append(qint16(11));      // ANIM.HQR index
    writer.append(quint8(0));       // no actions

    writer.append(quint8(LbaCharacter::OPANIM));
    writer.append(quint8(1));       // walk
    writer.append(quint8(8));
    writer.append(qint16(12));
    writer.append(QByteArray::fromHex("0101020304"));
    writer.append(quint8(LbaCharacter::OPEOF));

    LbaCharacter character;
    QVERIFY2(character.fromBuffer(writer.buffer()), qPrintable(character.errorString()));
    QVERIFY(character.isValid());
    QCOMPARE(character.bodies().count(), 1);
    QCOMPARE(character.animations().count(), 2);
    QVERIFY(character.body(0));
    QCOMPARE(character.body(0)->bodyIndex, 7);
    QVERIFY(character.body(0)->hasCollisionBox);
    QCOMPARE(character.body(0)->bottomLeft, QVector3D(-250, 0, -300));
    QCOMPARE(character.body(0)->topRight, QVector3D(250, 1200, 300));
    QVERIFY(character.animation(0));
    QCOMPARE(character.animation(0)->animationIndex, 11);
    QVERIFY(character.animation(1));
    QCOMPARE(character.animation(1)->animationIndex, 12);
    QCOMPARE(character.animation(1)->actions, QByteArray::fromHex("0101020304"));
    QCOMPARE(LbaCharacter::bodyName(0), QString("default"));
    QCOMPARE(LbaCharacter::bodyName(3), QString("body-3"));
    QCOMPARE(LbaCharacter::animationName(0), QString("idle"));
    QCOMPARE(LbaCharacter::animationName(1), QString("walk"));
    QCOMPARE(LbaCharacter::animationName(41), QString("animation-41"));

    QVERIFY(!character.fromBuffer(writer.buffer().left(writer.size()-1)));
    QVERIFY(!character.isValid());
    QVERIFY(character.bodies().isEmpty());
    QVERIFY(!character.errorString().isEmpty());
}

//-------------------------------------------------------------------------------------------
void TestLiblbadata::testLbaPalette()
{
    QVector<QRgb> palette(256);
    for (int i=0; i<256; i++) {
        palette[i] = QColor(i,qMin(i+1, 255), qMin(i+2, 255)).rgb();
    }

    LbaPalette in(palette);

    // Lba-Buffer
    {
        LbaPalette lbaPal;
        lbaPal.fromBuffer(in.toBuffer());
        QVERIFY(palette == lbaPal.palette());
    }

    // Gimp-Buffer
    {
        LbaPalette gimpPal;
        gimpPal.fromBuffer(in.toGimpPalette("test"));
        QVERIFY(palette == gimpPal.palette());
    }
}

//-------------------------------------------------------------------------------------------
QTEST_MAIN(TestLiblbadata)
