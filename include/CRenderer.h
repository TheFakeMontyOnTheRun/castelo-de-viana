#ifndef IRENDERER_H
#define IRENDERER_H

namespace RunTheWorld {

    struct Vec3 {
        Vec3(float aX, float aY, float aZ) : x(aX), y(aY), z(aZ) {
        }

        float x = 0;
        float y = 0;
        float z = 0;
    };

    struct Vec2 {
        Vec2(float aX, float aY) : x(aX), y(aY) {
        }

        float x = 0;
        float y = 0;
    };

    class CRenderer {
    private:
        CControlCallback mOnKeyPressedCallback;
        CControlCallback mOnKeyReleasedCallback;
     public:
        CRenderer( CControlCallback keyPressedCallback, CControlCallback keyReleasedCallback );
        virtual void render( const CGame& game, long ms );
	virtual void handleSystemEvents();
        virtual void sleep( long ms );
        virtual void fill(float x0, float x1, float y0, float x2, float x3, float y1, std::array<int,4> color);
        virtual void drawBackdropForHeading(int modulus, int zone);
    };
}
#endif
