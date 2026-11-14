#pragma once

// Administra una referencia COM y garantiza Release al salir del alcance.
// La clase expresa propiedad sin incorporar otro framework al proceso Explorer.
template <typename TipoInterfaz>
class PunteroCom {
  public:
    PunteroCom() = default;
    PunteroCom(const PunteroCom&) = delete;
    PunteroCom& operator=(const PunteroCom&) = delete;

    ~PunteroCom() { liberar(); }

    TipoInterfaz* obtener() const { return interfaz_; }
    TipoInterfaz* operator->() const { return interfaz_; }
    explicit operator bool() const { return interfaz_ != nullptr; }

    TipoInterfaz** recibir() {
        liberar();
        return &interfaz_;
    }

    void liberar() {
        if (!interfaz_) return;
        interfaz_->Release();
        interfaz_ = nullptr;
    }

  private:
    TipoInterfaz* interfaz_ = nullptr;
};
