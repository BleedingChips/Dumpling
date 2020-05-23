#pragma once
#include <DirectXMath.h>
#include <array>
namespace Dumpling::Dx
{

	template<typename StorageT, size_t ChannelCount> struct TypeStorage {
		static_assert(ChannelCount <= 4 && ChannelCount >= 1);
		std::array<StorageT, 1> operator()(size_t index) { assert(index < ChannelCount); return { Storage[index] }; }
		std::array<StorageT, 2> operator()(size_t index, size_t index2) { assert(index < ChannelCount); return { Storage[index], Storage[index2] }; }
		std::array<StorageT, 3> operator()(size_t index, size_t index2, size_t index3) { assert(index < ChannelCount); return { Storage[index], Storage[index2], Storage[index3] }; }
		std::array<StorageT, 4> operator()(size_t index, size_t index2, size_t index3, size_t index4) { assert(index < ChannelCount); return { Storage[index], Storage[index2], Storage[index3], Storage[index4] }; }
		TypeStorage operator+(TypeStorage const& ref) const noexcept { return Ope(ref, [](auto&& i, auto&& i2) { return i + i2; }); }
		TypeStorage& operator+=(TypeStorage const& ref) noexcept { return *this = *this + ref; }
		TypeStorage operator-(TypeStorage const& ref) const noexcept { return Ope(ref, [](auto&& i, auto&& i2) { return i - i2; }); }
		TypeStorage& operator-=(TypeStorage const& ref) noexcept { return *this = *this - ref; }
		TypeStorage operator*(TypeStorage const& ref) const noexcept { return Ope(ref, [](auto&& i, auto&& i2) { return i * i2; }); }
		TypeStorage& operator*=(TypeStorage const& ref) noexcept { return *this = *this * ref; }
		TypeStorage operator/(TypeStorage const& ref) const noexcept { return Ope(ref, [](auto&& i, auto&& i2) { return i / i2; }); }
		TypeStorage& operator/=(TypeStorage const& ref) noexcept { return *this = *this / ref; }
		operator std::enable_if_t<ChannelCount == 1, StorageT>() const noexcept { return Storage[0]; }
		TypeStorage(StorageT Input = 0) { for (size_t i = 0; i < ChannelCount; ++i) Storage[i] = Input; };
		TypeStorage(TypeStorage const& Input) : TypeStorage(static_cast<StorageT>(Input)) {}
		template<size_t Count, size_t Count2, typename = std::enable_if_t<Count + Count2 == ChannelCount>>
		TypeStorage(TypeStorage<StorageT, Count> const& Input1, TypeStorage<StorageT, Count2> const& Input2) {
			for (size_t i = 0; i < Count; ++i)
				Storage[i] = Input1.Storage[i];
			for (size_t i = 0; i < Count2; ++i)
				Storage[i + Count] = Input1.Storage[i];
		}
		template<size_t Count, size_t Count2, size_t Count3, typename = std::enable_if_t<Count + Count2 + Count3 == ChannelCount>>
		TypeStorage(TypeStorage<StorageT, Count> const& Input1, TypeStorage<StorageT, Count2> const& Input2, TypeStorage<StorageT, Count3> const& Input3) {
			for (size_t i = 0; i < Count; ++i)
				Storage[i] = Input1.Storage[i];
			for (size_t i = 0; i < Count2; ++i)
				Storage[i + Count] = Input2.Storage[i];
			for (size_t i = 0; i < Count3; ++i)
				Storage[i + Count2 + Count] = Input3.Storage[i];
		}
		template<typename OStorageT> operator TypeStorage<std::enable_if_t<std::is_convertible_v<StorageT, OStorageT>, OStorageT>, ChannelCount>() const noexcept {
			TypeStorage<OStorageT, ChannelCount> result;
			for (size_t i = 0; i < ChannelCount; ++i)
				result.Storage[i] = static_cast<OStorageT>(Storage[i]);
			return result;
		}
		size_t Size() const noexcept { return ChannelCount; }
		StorageT* Data() const noexcept { return Storage.data(); }
	private:
		template<typename FuncOpe> TypeStorage Ope(TypeStorage const& ref, FuncOpe&& FO) const noexcept {
			TypeStorage Result;
			for (size_t i = 0; i < ChannelCount; ++i)
				Result.Storage[i] = FO(Storage[i], ref.Storage[i]);
			return Result;
		}
		std::array<StorageT, ChannelCount> Storage;
	};

	template<typename StorageT, size_t ChannelCount> struct alignas(16) TypeStorageA : TypeStorage<StorageT, ChannelCount> {
		using TypeStorage<StorageT, ChannelCount>::TypeStorage;
		TypeStorageA(TypeStorageA const& ref) = default;
	};

	using XMVECTOR = DirectX::XMVECTOR;

	template<typename StorageT, size_t ChannelCount> XMVECTOR UseSIMD(TypeStorageA<StorageT, ChannelCount> const&);
	XMVECTOR ToSIMD(TypeStorageA<float, 1> const& Input) { return DirectX::XMLoadFloat(Input.Data()); }
	XMVECTOR ToSIMD(TypeStorageA<float, 2> const& Input) { return DirectX::XMLoadFloat2A(reinterpret_cast<DirectX::XMFLOAT2A const*>(Input.Data())); }
	XMVECTOR ToSIMD(TypeStorageA<float, 3> const& Input) { return DirectX::XMLoadFloat3A(reinterpret_cast<DirectX::XMFLOAT3A const*>(Input.Data())); }
	XMVECTOR ToSIMD(TypeStorageA<float, 4> const& Input) { return DirectX::XMLoadFloat4A(reinterpret_cast<DirectX::XMFLOAT4A const*>(Input.Data())); }
	XMVECTOR ToSIMD(TypeStorageA<int32_t, 1> const& Input) { return DirectX::XMLoadInt(reinterpret_cast<uint32_t const*>(Input.Data())); }
	XMVECTOR ToSIMD(TypeStorageA<int32_t, 2> const& Input) { return DirectX::XMLoadInt2(reinterpret_cast<uint32_t const*>(Input.Data())); }
	XMVECTOR ToSIMD(TypeStorageA<int32_t, 3> const& Input) { return DirectX::XMLoadInt3(reinterpret_cast<uint32_t const*>(Input.Data())); }
	XMVECTOR ToSIMD(TypeStorageA<int32_t, 4> const& Input) { return DirectX::XMLoadInt4(reinterpret_cast<uint32_t const*>(Input.Data())); }


	using Int = TypeStorage<int32_t, 1>;

}