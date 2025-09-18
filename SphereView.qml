import QtQuick 2.15
import QtQuick3D 1.15

Node {
    property vector3d spherePoint: Qt.vector3d(0, 0, 0)

    PerspectiveCamera {
        id: camera
        position: Qt.vector3d(0, 0, 600)
    }

    DirectionalLight {
        id: directionalLight
        brightness: 1
        eulerRotation.x: -30
        eulerRotation.y: -30
    }

    Model {
        id: sphere
        source: "#Sphere"
        scale: Qt.vector3d(100, 100, 100)
        materials: PrincipledMaterial {
            baseColor: "#808080"
            roughness: 0.5
            metalness: 0.2
        }
    }

    Model {
        id: point
        source: "#Sphere"
        scale: Qt.vector3d(5, 5, 5)
        position: spherePoint
        materials: PrincipledMaterial {
            baseColor: "#ff0000"
            roughness: 0.1
            metalness: 0.8
        }
    }

    OrbitCameraController {
        id: controller
        anchor: sphere
        camera: camera
    }

    // Coordinate system
    Model {
        id: xAxis
        source: "#Cylinder"
        position: Qt.vector3d(150, 0, 0)
        eulerRotation: Qt.vector3d(0, 0, -90)
        scale: Qt.vector3d(1, 300, 1)
        materials: PrincipledMaterial {
            baseColor: "#ff0000"
        }
    }

    Model {
        id: yAxis
        source: "#Cylinder"
        position: Qt.vector3d(0, 150, 0)
        scale: Qt.vector3d(1, 300, 1)
        materials: PrincipledMaterial {
            baseColor: "#00ff00"
        }
    }

    Model {
        id: zAxis
        source: "#Cylinder"
        position: Qt.vector3d(0, 0, 150)
        eulerRotation: Qt.vector3d(90, 0, 0)
        scale: Qt.vector3d(1, 300, 1)
        materials: PrincipledMaterial {
            baseColor: "#0000ff"
        }
    }
}
