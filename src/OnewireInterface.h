#pragma once

class OnewireInterface {
 public:
  OnewireInterface();
  virtual ~OnewireInterface();
  /*
   * Perform a 1-Wire reset cycle. Returns 1 if a device responds
   * with a presence pulse.  Returns 0 if there is no device or the
   * bus is shorted or otherwise held low for more than 250uS
   */
  virtual uint8_t reset(void) = 0;

  /*
   * Issues a 1-Wire rom select command, you do the reset first.
   */
  virtual void select(const uint8_t rom[8]) = 0;

  /*
   * Issues a 1-Wire rom skip command, to address all on bus.
   */
  virtual void skip(void) = 0;

  /*
   * Write a byte/sequence of bytes. If 'power' is one then the wire is held
   * high at
   * the end for parasitically powered devices. You are responsible
   * for eventually depowering it by calling depower() or doing
   * another read or write.
   */
  virtual void write(uint8_t v, uint8_t power = 0) = 0;
  virtual void write_bytes(const uint8_t *buf, uint16_t count,
                           bool power = 0) = 0;

  /*
   * Read a byte/sequence of bytes.
   */
  virtual uint8_t read(void) = 0;
  virtual void read_bytes(uint8_t *buf, uint16_t count) = 0;

  /*
   * Write a bit. The bus is always left powered at the end, see
   * note in write() about that.
   */
  virtual void write_bit(uint8_t v) = 0;

  /*
   * Read a bit.
   */
  virtual uint8_t read_bit(void) = 0;

  /*
   * Stop forcing power onto the bus. You only need to do this if
   * you used the 'power' flag to write() or used a write_bit() call
   * and aren't about to do another read or write. You would rather
   * not leave this powered if you don't have to, just in case
   * someone shorts your bus.
   */
  virtual void depower(void) = 0;

  /*
   * Clear the search state so that if will start from the beginning again.
   */
  virtual void reset_search() = 0;

  /*
   * Setup the search to find the device type 'family_code' on the next call
   * to search(*newAddr) if it is present.
   */
  virtual void target_search(uint8_t family_code) = 0;

  /*
   * Look for the next device. Returns 1 if a new address has been
   * returned. A zero might mean that the bus is shorted, there are
   * no devices, or you have already retrieved all of them.  It
   * might be a good idea to check the CRC to make sure you didn't
   * get garbage.  The order is deterministic. You will always get
   * the same devices in the same order.
   */
  virtual uint8_t search(uint8_t *newAddr, bool search_mode = true) = 0;
};
