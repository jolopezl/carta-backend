/* This file is part of the CARTA Image Viewer: https://github.com/CARTAvis/carta-backend
   Copyright 2018, 2019, 2020, 2021 Academia Sinica Institute of Astronomy and Astrophysics (ASIAA),
   Associated Universities, Inc. (AUI) and the Inter-University Institute for Data Intensive Astronomy (IDIA)
   SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef CARTA_BACKEND_IMAGEDATA_CARTACASAIMAGE_TCC_
#define CARTA_BACKEND_IMAGEDATA_CARTACASAIMAGE_TCC_

#include <casacore/images/Images/SubImage.h>
#include <casacore/lattices/Lattices/MaskedLatticeIterator.h>

#include "Util/Casacore.h"

using namespace carta;

template <class T>
CartaCasaImage<T>::CartaCasaImage(const std::string& filename) : casacore::ImageInterface<float>(), _filename(filename) {
    switch (CasacoreImageType(filename)) {
        case casacore::ImageOpener::AIPSPP:
            _image.reset(new casacore::PagedImage<T>(filename));
            break;
        default:
            spdlog::error("Unknown image type!");
            break;
    }
    setCoordinateInfo(_image->coordinates());
}

template <class T>
CartaCasaImage<T>::CartaCasaImage(const CartaCasaImage& other) : casacore::ImageInterface<float>(other), _filename(other._filename) {
    switch (CasacoreImageType(other._filename)) {
        case casacore::ImageOpener::AIPSPP:
            _image.reset(new casacore::PagedImage<T>(other._filename));
            break;
        default:
            spdlog::error("Unknown image type!");
            break;
    }
    setCoordinateInfo(_image->coordinates());
}

template <class T>
CartaCasaImage<T>::~CartaCasaImage() {}

// Image interface

template <class T>
casacore::String CartaCasaImage<T>::imageType() const {
    return "CartaCasaImage";
}

template <class T>
casacore::String CartaCasaImage<T>::name(bool stripPath) const {
    return _image->name();
}

template <class T>
casacore::IPosition CartaCasaImage<T>::shape() const {
    return _image->shape();
}

template <class T>
casacore::Bool CartaCasaImage<T>::ok() const {
    return _image->ok();
}

template <class T>
casacore::DataType CartaCasaImage<T>::dataType() const {
    return casacore::DataType::TpFloat;
}

template <class T>
casacore::Bool CartaCasaImage<T>::doGetSlice(casacore::Array<float>& buffer, const casacore::Slicer& section) {
    if constexpr (std::is_same_v<T, casacore::Int>) {
        casacore::SubImage<casacore::Int> sub_image(*_image, section);
        casacore::RO_MaskedLatticeIterator<casacore::Int> lattice_iter(sub_image);
        for (lattice_iter.reset(); !lattice_iter.atEnd(); ++lattice_iter) {
            casacore::Array<float> cursor_data = (const casacore::Array<float>&)lattice_iter.cursor();
            casacore::IPosition cursor_shape(lattice_iter.cursorShape());
            casacore::IPosition cursor_position(lattice_iter.position());
            casacore::Slicer cursor_slicer(cursor_position, cursor_shape);
            buffer(cursor_slicer) = cursor_data;
        }
    } else if constexpr (std::is_same_v<T, casacore::Bool>) {
        casacore::SubImage<casacore::Bool> sub_image(*_image, section);
        casacore::RO_MaskedLatticeIterator<casacore::Bool> lattice_iter(sub_image);
        for (lattice_iter.reset(); !lattice_iter.atEnd(); ++lattice_iter) {
            casacore::Array<casacore::Bool> cursor_data = lattice_iter.cursor();
            casacore::IPosition cursor_shape(lattice_iter.cursorShape());
            casacore::IPosition cursor_position(lattice_iter.position());
            casacore::Slicer cursor_slicer(cursor_position, cursor_shape);

            // represent the bool type as the float type
            casacore::Array<float> cursor_data_float(cursor_shape);
            auto* bool_ptr = cursor_data.data();
            auto* float_ptr = cursor_data_float.data();
            for (casacore::uInt i = 0; i < cursor_data.size(); ++i) {
                if (bool_ptr[i]) {
                    float_ptr[i] = 1.0;
                } else {
                    float_ptr[i] = 0.0;
                }
            }
            buffer(cursor_slicer) = cursor_data_float;
        }
    } else {
        spdlog::error("Image data type is not supported!");
        return false;
    }
    return true;
}

template <class T>
void CartaCasaImage<T>::doPutSlice(
    const casacore::Array<float>& buffer, const casacore::IPosition& where, const casacore::IPosition& stride) {
    throw(casacore::AipsError("CartaCasaImage::doPutSlice - image is not writable"));
}

template <class T>
const casacore::LatticeRegion* CartaCasaImage<T>::getRegionPtr() const {
    return nullptr;
}

template <class T>
casacore::ImageInterface<float>* CartaCasaImage<T>::cloneII() const {
    return new CartaCasaImage(*this);
}

template <class T>
void CartaCasaImage<T>::resize(const casacore::TiledShape& newShape) {
    throw(casacore::AipsError("CartaCasaImage::resize - image is not writable"));
}

template <class T>
casacore::uInt CartaCasaImage<T>::advisedMaxPixels() const {
    return _image->advisedMaxPixels();
}

template <class T>
casacore::IPosition CartaCasaImage<T>::doNiceCursorShape(casacore::uInt) const {
    return _image->doNiceCursorShape(_image->advisedMaxPixels());
}

template <class T>
casacore::Bool CartaCasaImage<T>::isMasked() const {
    return _image->isMasked();
}

template <class T>
casacore::Bool CartaCasaImage<T>::hasPixelMask() const {
    return _image->hasPixelMask();
}

template <class T>
const casacore::Lattice<bool>& CartaCasaImage<T>::pixelMask() const {
    return _image->pixelMask();
}

template <class T>
casacore::Lattice<bool>& CartaCasaImage<T>::pixelMask() {
    return _image->pixelMask();
}

template <class T>
casacore::Bool CartaCasaImage<T>::doGetMaskSlice(casacore::Array<bool>& buffer, const casacore::Slicer& section) {
    return _image->doGetMaskSlice(buffer, section);
}

#endif // CARTA_BACKEND_IMAGEDATA_CARTACASAIMAGE_TCC_
