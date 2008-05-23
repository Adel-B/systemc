/*****************************************************************************

  The following code is derived, directly or indirectly, from the SystemC
  source code Copyright (c) 1996-2007 by all Contributors.
  All Rights reserved.

  The contents of this file are subject to the restrictions and limitations
  set forth in the SystemC Open Source License Version 3.0 (the "License");
  You may not use this file except in compliance with such restrictions and
  limitations. You may obtain instructions on how to receive a copy of the
  License at http://www.systemc.org/. Software distributed by Contributors
  under the License is distributed on an "AS IS" basis, WITHOUT WARRANTY OF
  ANY KIND, either express or implied. See the License for the specific
  language governing rights and limitations under the License.

 *****************************************************************************/

#ifndef __SIMPLE_INITIATOR_SOCKET_H__
#define __SIMPLE_INITIATOR_SOCKET_H__

#include "tlm.h"

namespace tlm_utils {

template <typename MODULE,
          unsigned int BUSWIDTH = 32,
          typename TYPES = tlm::tlm_base_protocol_types>
class simple_initiator_socket :
  public tlm::tlm_initiator_socket<BUSWIDTH, TYPES>
{
public:
  typedef typename TYPES::tlm_payload_type              transaction_type;
  typedef typename TYPES::tlm_phase_type                phase_type;
  typedef tlm::tlm_sync_enum                            sync_enum_type;
  typedef tlm::tlm_fw_transport_if<TYPES>               fw_interface_type;
  typedef tlm::tlm_bw_transport_if<TYPES>               bw_interface_type;
  typedef tlm::tlm_initiator_socket<BUSWIDTH, TYPES>    base_type;

public:
  explicit simple_initiator_socket(const char* n = "simple_initiator_socket") :
    base_type(sc_core::sc_gen_unique_name(n)),
    m_process(this->name())
  {
    this->m_export.bind(m_process);
  }

  void register_nb_transport_bw(MODULE* mod,
                                sync_enum_type (MODULE::*cb)(transaction_type&,
                                                             phase_type&,
                                                             sc_core::sc_time&))
  {
    m_process.set_transport_ptr(mod, cb);
  }

  void register_invalidate_direct_mem_ptr(MODULE* mod,
                                          void (MODULE::*cb)(sc_dt::uint64, sc_dt::uint64))
  {
    m_process.set_invalidate_direct_mem_ptr(mod, cb);
  }

private:
  class process : public tlm::tlm_bw_transport_if<TYPES>
  {
  public:
    typedef sync_enum_type (MODULE::*TransportPtr)(transaction_type&,
                                                   tlm::tlm_phase&,
                                                   sc_core::sc_time&);
    typedef void (MODULE::*InvalidateDirectMemPtr)(sc_dt::uint64,
                                                   sc_dt::uint64);
      
    process(const std::string& name) :
      m_name(name),
      m_mod(0),
      m_transport_ptr(0),
      m_invalidate_direct_mem_ptr(0)
    {
    }
  
    void set_transport_ptr(MODULE* mod, TransportPtr p)
    {
      if (m_transport_ptr) {
        std::cerr << m_name << ": non-blocking callback allready registered" << std::endl;

      } else {
        assert(!m_mod || m_mod == mod);
        m_mod = mod;
        m_transport_ptr = p;
      }
    }

    void set_invalidate_direct_mem_ptr(MODULE* mod, InvalidateDirectMemPtr p)
    {
      if (m_invalidate_direct_mem_ptr) {
        std::cerr << m_name << ": invalidate DMI callback allready registered" << std::endl;

      } else {
        assert(!m_mod || m_mod == mod);
        m_mod = mod;
        m_invalidate_direct_mem_ptr = p;
      }
    }

    sync_enum_type nb_transport_bw(transaction_type& trans, phase_type& phase, sc_core::sc_time& t)
    {
      if (m_transport_ptr) {
        // forward call
        assert(m_mod);
        return (m_mod->*m_transport_ptr)(trans, phase, t);

      } else {
        std::cerr << m_name << ": no transport callback registered" << std::endl;
        assert(0); exit(1);
      }
    }

    void invalidate_direct_mem_ptr(sc_dt::uint64 start_range,
                                   sc_dt::uint64 end_range)
    {
      if (m_invalidate_direct_mem_ptr) {
        // forward call
        assert(m_mod);
        (m_mod->*m_invalidate_direct_mem_ptr)(start_range, end_range);
      }
    }

  private:
    const std::string m_name;
    MODULE* m_mod;
    TransportPtr m_transport_ptr;
    InvalidateDirectMemPtr m_invalidate_direct_mem_ptr;
  };

private:
  process m_process;
};

// Tagged version

template <typename MODULE,
          unsigned int BUSWIDTH = 32,
          typename TYPES = tlm::tlm_base_protocol_types>
class simple_initiator_socket_tagged :
  public tlm::tlm_initiator_socket<BUSWIDTH, TYPES>
{
public:
  typedef typename TYPES::tlm_payload_type              transaction_type;
  typedef typename TYPES::tlm_phase_type                phase_type;
  typedef tlm::tlm_sync_enum                            sync_enum_type;
  typedef tlm::tlm_fw_transport_if<TYPES>               fw_interface_type;
  typedef tlm::tlm_bw_transport_if<TYPES>               bw_interface_type;
  typedef tlm::tlm_initiator_socket<BUSWIDTH, TYPES>    base_type;

public:
  explicit simple_initiator_socket_tagged(const char* n = "simple_initiator_socket_tagged") :
    base_type(sc_core::sc_gen_unique_name(n)),
    m_process(this->name())
  {
    this->m_export.bind(m_process);
  }

  void register_nb_transport_bw(MODULE* mod,
                                sync_enum_type (MODULE::*cb)(int,
                                                             transaction_type&,
                                                             phase_type&,
                                                             sc_core::sc_time&),
                                int id)
  {
    m_process.set_transport_ptr(mod, cb);
    m_process.set_transport_user_id(id);
  }

  void register_invalidate_direct_mem_ptr(MODULE* mod,
                                          void (MODULE::*cb)(int, sc_dt::uint64, sc_dt::uint64),
                                           int id)
  {
    m_process.set_invalidate_direct_mem_ptr(mod, cb);
    m_process.set_invalidate_dmi_user_id(id);
  }

private:
  class process : public tlm::tlm_bw_transport_if<TYPES>
  {
  public:
    typedef sync_enum_type (MODULE::*TransportPtr)(int,
                                                   transaction_type&,
                                                   tlm::tlm_phase&,
                                                   sc_core::sc_time&);
    typedef void (MODULE::*InvalidateDirectMemPtr)(int,
                                                   sc_dt::uint64,
                                                   sc_dt::uint64);
      
    process(const std::string& name) :
      m_name(name),
      m_mod(0),
      m_transport_ptr(0),
      m_invalidate_direct_mem_ptr(0),
      m_transport_user_id(0),
      m_invalidate_direct_mem_user_id(0)
    {
    }
  
    void set_transport_user_id(int id) { m_transport_user_id = id; }
    void set_invalidate_dmi_user_id(int id) { m_invalidate_direct_mem_user_id = id; }

    void set_transport_ptr(MODULE* mod, TransportPtr p)
    {
      if (m_transport_ptr) {
        std::cerr << m_name << ": non-blocking callback allready registered" << std::endl;

      } else {
        assert(!m_mod || m_mod == mod);
        m_mod = mod;
        m_transport_ptr = p;
      }
    }

    void set_invalidate_direct_mem_ptr(MODULE* mod, InvalidateDirectMemPtr p)
    {
      if (m_invalidate_direct_mem_ptr) {
        std::cerr << m_name << ": invalidate DMI callback allready registered" << std::endl;

      } else {
        assert(!m_mod || m_mod == mod);
        m_mod = mod;
        m_invalidate_direct_mem_ptr = p;
      }
    }

    sync_enum_type nb_transport_bw(transaction_type& trans, phase_type& phase, sc_core::sc_time& t)
    {
      if (m_transport_ptr) {
        // forward call
        assert(m_mod);
        return (m_mod->*m_transport_ptr)(m_transport_user_id, trans, phase, t);

      } else {
        std::cerr << m_name << ": no transport callback registered" << std::endl;
        assert(0); exit(1);
      }
    }

    void invalidate_direct_mem_ptr(sc_dt::uint64 start_range,
                                   sc_dt::uint64 end_range)
    {
      if (m_invalidate_direct_mem_ptr) {
        // forward call
        assert(m_mod);
        (m_mod->*m_invalidate_direct_mem_ptr)(m_invalidate_direct_mem_user_id, start_range, end_range);
      }
    }

  private:
    const std::string m_name;
    MODULE* m_mod;
    TransportPtr m_transport_ptr;
    InvalidateDirectMemPtr m_invalidate_direct_mem_ptr;
    int m_transport_user_id;
    int m_invalidate_direct_mem_user_id;
  };

private:
  process m_process;
};

}

#endif
