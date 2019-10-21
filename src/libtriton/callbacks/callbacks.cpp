//! \file
/*
**  Copyright (C) - Triton
**
**  This program is under the terms of the BSD License.
*/

#include <triton/api.hpp>
#include <triton/callbacks.hpp>
#include <triton/exceptions.hpp>



namespace triton {
  namespace callbacks {

    Callbacks::Callbacks(triton::API& api) : api(api) {
      this->isDefined = false;
      this->mget      = false;
      this->mload     = false;
      this->mput      = false;
      this->mstore    = false;
    }


    void Callbacks::addCallback(triton::callbacks::getConcreteMemoryValueCallback cb) {
      this->getConcreteMemoryValueCallbacks.push_back(cb);
      this->isDefined = true;
    }


    void Callbacks::addCallback(triton::callbacks::getConcreteRegisterValueCallback cb) {
      this->getConcreteRegisterValueCallbacks.push_back(cb);
      this->isDefined = true;
    }


    void Callbacks::addCallback(triton::callbacks::setConcreteMemoryValueCallback cb) {
      this->setConcreteMemoryValueCallbacks.push_back(cb);
      this->isDefined = true;
    }


    void Callbacks::addCallback(triton::callbacks::setConcreteRegisterValueCallback cb) {
      this->setConcreteRegisterValueCallbacks.push_back(cb);
      this->isDefined = true;
    }


    void Callbacks::addSymbolicNodeSimplificationCallback(triton::callbacks::symbolicSimplificationCallback cb) {
      this->symbolicNodeSimplificationCallbacks.push_back(cb);
      this->isDefined = true;
    }


    void Callbacks::addSymbolicTreeSimplificationCallback(triton::callbacks::symbolicSimplificationCallback cb) {
      this->symbolicTreeSimplificationCallbacks.push_back(cb);
      this->isDefined = true;
    }


    void Callbacks::removeAllCallbacks(void) {
      this->getConcreteMemoryValueCallbacks.clear();
      this->getConcreteRegisterValueCallbacks.clear();
      this->setConcreteMemoryValueCallbacks.clear();
      this->setConcreteRegisterValueCallbacks.clear();
      this->symbolicNodeSimplificationCallbacks.clear();
      this->symbolicTreeSimplificationCallbacks.clear();
    }


    void Callbacks::removeCallback(triton::callbacks::getConcreteMemoryValueCallback cb) {
      this->getConcreteMemoryValueCallbacks.remove(cb);
      if (this->countCallbacks() == 0)
        this->isDefined = false;
    }


    void Callbacks::removeCallback(triton::callbacks::getConcreteRegisterValueCallback cb) {
      this->getConcreteRegisterValueCallbacks.remove(cb);
      if (this->countCallbacks() == 0)
        this->isDefined = false;
    }


    void Callbacks::removeCallback(triton::callbacks::setConcreteMemoryValueCallback cb) {
      this->setConcreteMemoryValueCallbacks.remove(cb);
      if (this->countCallbacks() == 0)
        this->isDefined = false;
    }


    void Callbacks::removeCallback(triton::callbacks::setConcreteRegisterValueCallback cb) {
      this->setConcreteRegisterValueCallbacks.remove(cb);
      if (this->countCallbacks() == 0)
        this->isDefined = false;
    }


    void Callbacks::removeSymbolicNodeSimplificationCallback(triton::callbacks::symbolicSimplificationCallback cb) {
      this->symbolicNodeSimplificationCallbacks.remove(cb);
      if (this->countCallbacks() == 0)
        this->isDefined = false;
    }


    void Callbacks::removeSymbolicTreeSimplificationCallback(triton::callbacks::symbolicSimplificationCallback cb) {
      this->symbolicTreeSimplificationCallbacks.remove(cb);
      if (this->countCallbacks() == 0)
        this->isDefined = false;
    }


    triton::ast::SharedAbstractNode Callbacks::processCallbacks(triton::callbacks::callback_e kind, triton::ast::SharedAbstractNode node) {
      switch (kind) {
        case triton::callbacks::SYMBOLIC_NODE_SIMPLIFICATION: {
          for (auto& function: this->symbolicNodeSimplificationCallbacks) {
            // Reinject node in next callback
            node = function(this->api, node);
            if (node == nullptr)
              throw triton::exceptions::Callbacks("Callbacks::processCallbacks(SYMBOLIC_NODE_SIMPLIFICATION): You cannot return a nullptr node.");
          }
          break;
        }
        case triton::callbacks::SYMBOLIC_TREE_SIMPLIFICATION: {
          for (auto& function: this->symbolicTreeSimplificationCallbacks) {
            // Reinject node in next callback
            node = function(this->api, node);
            if (node == nullptr)
              throw triton::exceptions::Callbacks("Callbacks::processCallbacks(SYMBOLIC_TREE_SIMPLIFICATION): You cannot return a nullptr node.");
          }
          break;
        }
        default:
          throw triton::exceptions::Callbacks("Callbacks::processCallbacks(): Invalid kind of callback for this C++ polymorphism.");
      };

      return node;
    }


    void Callbacks::processCallbacks(triton::callbacks::callback_e kind, const triton::arch::MemoryAccess& mem) {
      switch (kind) {
        case triton::callbacks::GET_CONCRETE_MEMORY_VALUE: {
          /* Check if we are already in the callback to avoid infinite recursion */
          if (this->mload) {
            break;
          }

          for (auto& function: this->getConcreteMemoryValueCallbacks) {
            this->mload = true;
            function(this->api, mem);
            if (mem.getLeaAst() != nullptr)
                this->api.getSymbolicEngine()->initLeaAst(const_cast<triton::arch::MemoryAccess&>(mem), true);
            this->mload = false;
          }

          break;
        }

        default:
          throw triton::exceptions::Callbacks("Callbacks::processCallbacks(): Invalid kind of callback for this C++ polymorphism.");
      };
    }


    void Callbacks::processCallbacks(triton::callbacks::callback_e kind, const triton::arch::Register& reg) {
      switch (kind) {
        case triton::callbacks::GET_CONCRETE_REGISTER_VALUE: {
          /* Check if we are already in the callback to avoid infinite recursion */
          if (this->mget) {
            break;
          }

          for (auto& function: this->getConcreteRegisterValueCallbacks) {
            this->mget = true;
            function(this->api, reg);
            this->mget = false;
          }

          break;
        }

        default:
          throw triton::exceptions::Callbacks("Callbacks::processCallbacks(): Invalid kind of callback for this C++ polymorphism.");
      };
    }


    void Callbacks::processCallbacks(triton::callbacks::callback_e kind, const triton::arch::MemoryAccess& mem, const triton::uint512& value) {
      switch (kind) {
        case triton::callbacks::SET_CONCRETE_MEMORY_VALUE: {
          /* Check if we are already in the callback to avoid infinite recursion */
          if (this->mstore) {
            break;
          }

          for (auto& function: this->setConcreteMemoryValueCallbacks) {
            this->mstore = true;
            function(this->api, mem, value);
            this->mstore = false;
          }

          break;
        }

        default:
          throw triton::exceptions::Callbacks("Callbacks::processCallbacks(): Invalid kind of callback for this C++ polymorphism.");
      };
    }


    void Callbacks::processCallbacks(triton::callbacks::callback_e kind, const triton::arch::Register& reg, const triton::uint512& value) {
      switch (kind) {
        case triton::callbacks::SET_CONCRETE_REGISTER_VALUE: {
          /* Check if we are already in the callback to avoid infinite recursion */
          if (this->mput) {
            break;
          }

          for (auto& function: this->setConcreteRegisterValueCallbacks) {
            this->mput = true;
            function(this->api, reg, value);
            this->mput = false;
          }

          break;
        }

        default:
          throw triton::exceptions::Callbacks("Callbacks::processCallbacks(): Invalid kind of callback for this C++ polymorphism.");
      };
    }


    bool Callbacks::hasRegisteredCallback(triton::callbacks::callback_e kind) const {
      switch (kind) {
        case GET_CONCRETE_MEMORY_VALUE:
          return !getConcreteMemoryValueCallbacks.empty();
        case GET_CONCRETE_REGISTER_VALUE:
          return !getConcreteRegisterValueCallbacks.empty();
        case SET_CONCRETE_MEMORY_VALUE:
          return !getConcreteMemoryValueCallbacks.empty();
        case SET_CONCRETE_REGISTER_VALUE:
          return !setConcreteRegisterValueCallbacks.empty();
        case SYMBOLIC_NODE_SIMPLIFICATION:
          return !symbolicNodeSimplificationCallbacks.empty();
        case SYMBOLIC_TREE_SIMPLIFICATION:
          return !symbolicTreeSimplificationCallbacks.empty();
        default:
          throw triton::exceptions::Callbacks("Callbacks::hasCallbacks(): Unknown kind of callback.");
      }
      return false;
    }


    triton::usize Callbacks::countCallbacks(void) const {
      triton::usize count = 0;

      count += this->getConcreteMemoryValueCallbacks.size();
      count += this->getConcreteRegisterValueCallbacks.size();
      count += this->setConcreteMemoryValueCallbacks.size();
      count += this->setConcreteRegisterValueCallbacks.size();
      count += this->symbolicNodeSimplificationCallbacks.size();
      count += this->symbolicTreeSimplificationCallbacks.size();

      return count;
    }

  }; /* callbacks namespace */
}; /* triton namespace */
