#pragma once
class MyCamera2D : public BasicCamera2D
{
public:

	/// @brief デフォルトコンストラクタ
	SIV3D_NODISCARD_CXX20
		MyCamera2D() = default;

	/// @brief 2D カメラを作成します。
	/// @param center カメラが見ている中心座標の初期設定
	/// @param scale カメラのズーム倍率の初期設定
	/// @param cameraControl カメラの操作オプション
	SIV3D_NODISCARD_CXX20
		MyCamera2D(Vec2 center, double scale = 1.0, CameraControl cameraControl = CameraControl::Default) noexcept;

	/// @brief 2D カメラを作成します。
	/// @param center カメラが見ている中心座標の初期設定
	/// @param scale カメラのズーム倍率の初期設定
	/// @param parameters カメラの操作パラメータ
	SIV3D_NODISCARD_CXX20
		MyCamera2D(Vec2 center, double scale, const Camera2DParameters& parameters) noexcept;

	/// @brief カメラの操作パラメータを変更します。
	/// @param parameters 新しいカメラの操作パラメータ
	void setParameters(const Camera2DParameters& parameters) noexcept;

	/// @brief 現在のカメラの操作パラメータを返します。
	/// @return 現在のカメラの操作パラメータ
	[[nodiscard]]
	const Camera2DParameters& getParameters() const noexcept;

	/// @brief カメラが見る中心座標の目標を設定します。
	/// @param targetCenter カメラが見る中心座標の目標
	/// @remark カメラは一定の時間をかけて目標の座標に移動します。
	void setTargetCenter(Vec2 targetCenter) noexcept;

	/// @brief 現在の目標中心座標を返します。
	/// @return 現在の目標中心座標
	const Vec2& getTargetCenter() const noexcept;

	/// @brief カメラのズームアップ倍率の目標を設定します。
	/// @param targetScale カメラのズーム倍率の目標
	/// @remark カメラは一定の時間をかけて目標のズーム倍率になります。
	void setTargetScale(double targetScale) noexcept;

	/// @brief 現在の目標ズームアップ倍率を返します。
	/// @return 現在の目標ズームアップ倍率
	double getTargetScale() const noexcept;

	/// @brief 指定した中心座標とズーム倍率を即座に適用します。
	/// @param center カメラが見る中心座標
	/// @param scale カメラのズーム倍率
	void jumpTo(Vec2 center, double scale) noexcept;

	/// @brief 2D カメラの状態を更新します。
	/// @param deltaTime 前回のフレームからの経過時間（秒）
	/// @param sceneSize レンダーターゲットのサイズ（ピクセル）
	void update(double deltaTime = Scene::DeltaTime(), SizeF sceneSize = Graphics2D::GetRenderTargetSize());

	void updateWheel(const SizeF& sceneSize = Graphics2D::GetRenderTargetSize());

	// 指定したスクリーン座標を拡大・縮小の支点にしてズームする
	void zoomAt(const Vec2& screenPos, double targetScale, const SizeF& sceneSize = Graphics2D::GetRenderTargetSize());

	/// @brief 右クリックによる移動の開始座標を返します。
	/// @return 右クリックによる移動の開始座標。右クリックによる移動が開始されていない場合は none
	[[nodiscard]]
	const Optional<Vec2>& getGrabbedPos() const noexcept;

protected:

	double m_targetScale = BasicCamera2D::m_scale;

	double m_scaleChangeVelocity = 0.0;

	Vec2 m_targetCenter = BasicCamera2D::m_center;

	Vec2 m_positionChangeVelocity = Vec2::Zero();

	Optional<Vec2> m_grabbedPos;

	Optional<std::pair<Vec2, Vec2>> m_pointedScale;

	Camera2DParameters m_parameters;

	
};

inline MyCamera2D::MyCamera2D(const Vec2 center, const double scale, const CameraControl cameraControl) noexcept
	: MyCamera2D{ center, scale, Camera2DParameters::Make(cameraControl) } {
}

inline MyCamera2D::MyCamera2D(const Vec2 center, const double scale, const Camera2DParameters& parameters) noexcept
	: BasicCamera2D{ center, scale }
	, m_parameters{ parameters } {
}

inline void MyCamera2D::setParameters(const Camera2DParameters& parameters) noexcept
{
	m_parameters = parameters;
}

inline const Camera2DParameters& MyCamera2D::getParameters() const noexcept
{
	return m_parameters;
}

inline void MyCamera2D::setTargetCenter(const Vec2 targetCenter) noexcept
{
	m_grabbedPos.reset();
	m_pointedScale.reset();
	m_targetCenter = targetCenter;
}

inline const Vec2& MyCamera2D::getTargetCenter() const noexcept
{
	return m_targetCenter;
}

inline void MyCamera2D::setTargetScale(const double targetScale) noexcept
{
	m_grabbedPos.reset();
	m_pointedScale.reset();
	m_targetScale = targetScale;
}

inline double MyCamera2D::getTargetScale() const noexcept
{
	return m_targetScale;
}

inline void MyCamera2D::jumpTo(const Vec2 center, const double scale) noexcept
{
	m_grabbedPos.reset();
	m_pointedScale.reset();
	m_targetCenter = m_center = center;
	m_targetScale = m_scale = scale;
	m_positionChangeVelocity = Vec2::Zero();
	m_scaleChangeVelocity = 0.0;
}

inline const Optional<Vec2>& MyCamera2D::getGrabbedPos() const noexcept
{
	return m_grabbedPos;
}
