#ifndef MESSAGECOMPOSER_H
#define MESSAGECOMPOSER_H

#include <QByteArray>

class GJson;

namespace MessageComposer
{
  quint32 crc32(quint32 crc, const void* data, size_t data_len);
  inline quint32 crc32(const void* data, size_t data_len)
    { return crc32(0, data, data_len); }
  inline quint32 crc32(const QByteArray& data)
    { return crc32(0, data.constData(), (size_t) data.size()); }

/*
 * General Message Format
 * ======================
 * +0 Flags [2 bytes] - Uncompressed length and flags
 * +2 Payload - Message data (depends of message type)
 * + CRC32 (2 + payload size) [4 bytes] CRC32 of Flags+Payload
 *
 * Field Flags
 * ===========
 * Bit 15 (most significant) - 1 if message is compressed
 * Bit 14                    - 1 if message is encryped
 * Bits 0-13 - uncompressd data size (0 if bit 15 is not set)
 *
 * Payload for non-encryped message is MSGPACK-ed Json (compressed or non),
 * depends of bit 14.
 *
 * For encryped message uncompressed size specified only for encryped part
 * of payload (if this part is exist).
 * For specifications of encryped data see comments in file
 * encrypted_message.cpp
 */

  /* Composing */
  QByteArray composeNonEncrypted(const GJson& msg);
  QByteArray composeEncrypted(const QByteArray& data, size_t uncompressed_size);

  /* Uncomposing */
  bool isValid(const QByteArray& msg);
  bool isEncrypted(const QByteArray& msg);
  GJson uncomposeNonEncrypted(const QByteArray& msg);
  QByteArray uncomposeEncrypted(const QByteArray& msg, int& uncompressed_size);
}

#endif // MESSAGECOMPOSER_H
