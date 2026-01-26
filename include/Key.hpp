#ifndef KEY_HPP
#define KEY_HPP

class Key
{
  public:
    Key() : m_vk(0), m_sc(0), m_ctrl(false), m_alt(false), m_shift(false) {}

    explicit Key(unsigned int vk, unsigned int sc, bool ctrl = false, bool alt = false, bool shift = false)
        : m_vk(vk), m_sc(sc), m_ctrl(ctrl), m_alt(alt), m_shift(shift)
    {
    }

    [[nodiscard]] unsigned int get_virtual_code() const { return m_vk; }
    [[nodiscard]] unsigned int get_scan_code() const { return m_sc; }
    [[nodiscard]] bool get_control() const { return m_ctrl; }
    [[nodiscard]] bool get_alt() const { return m_alt; }
    [[nodiscard]] bool get_shift() const { return m_shift; }

    void set_virtual_code(unsigned int vk) { m_vk = vk; }
    void set_scan_code(unsigned int sc) { m_sc = sc; }
    void set_control(bool ctrl) { m_ctrl = ctrl; }
    void set_alt(bool alt) { m_alt = alt; }
    void set_shift(bool shift) { m_shift = shift; }

  private:
    unsigned int m_vk;
    unsigned int m_sc;
    bool m_ctrl;
    bool m_alt;
    bool m_shift;
};

#endif // KEY_HPP
