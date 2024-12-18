#include <iostream>
#include <SFML/Graphics.hpp>
#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>
#include <thread>
#include <string>
#include <atomic>
#include <msclr/marshal_cppstd.h>
#include <mutex>
#include <SFML/Audio.hpp>
#pragma warning(disable: 4996)
#pragma once
namespace test {
	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;
	using namespace std;
	using namespace sf;
	int p2movement = 0;
	float p2jump = 0;
	struct HeroUpdate {
		float positionX, positionY;
		int health, dir, AnimX, AnimY;
	};
	SOCKET newConnection;
    public class Hero {
    public:
        float positionX, positionY;
        float scale, dx = 0, dy = 0, speed = 0;
        int dir = 1, health = 50, AnimX, AnimY;
        int AnimStatus = 0;
        float IdleFrame = 0, RunFrame = 0, JumpFrame = 0, AtackFrame, LastAttack = 0, AttackFrame = 0, JumpTime = 0;
        string file;
        sf::Image image;
        Texture texture;
        Sprite sprite;
        bool IsAlive = true;
        bool isGrounded = true;
        bool canDoubleJump = false;
        bool jumpKeyPressed = false;
        Hero(string F, float X, float Y, float Scale) {
            file = F;
            scale = Scale;
            image.loadFromFile("images/" + file);
            texture.loadFromImage(image);
            sprite.setTexture(texture);
            positionX = X;
            positionY = Y;
            sprite.setTextureRect(IntRect(0, 0, 64, 64));
            sprite.setScale(scale, scale);
        }
        void DrawAnim() {
            if (dir == 0) {
                sprite.setTextureRect(IntRect(AnimX, AnimY, -64, 64));
            }
            else {
                sprite.setTextureRect(IntRect(AnimX, AnimY, 64, 64));
            }
        }
        void SetAnim(int x, int y) {
            int temp;
            switch (x) {
            case 0:
                temp = int(IdleFrame) % 5;
                if (y == 0) {
                    AnimX = 64 + 64 * temp;
                    AnimY = 0;
                    //sprite.setTextureRect(IntRect(64 + 64 * temp, 0, -64, 64));
                }
                else {
                    AnimX = 64 * temp;
                    AnimY = 0;
                    //sprite.setTextureRect(IntRect(64 * temp, 0, 64, 64));
                }
                break;
            case 1:
                temp = int(RunFrame) % 8;
                if (y == 0) {
                    AnimX = 64 + 64 * temp;
                    AnimY = 64;
                    //sprite.setTextureRect(IntRect(64 + 64 * temp, 64, -64, 64));
                }
                else {
                    AnimX = 64 * temp;
                    AnimY = 64;
                    //sprite.setTextureRect(IntRect(64 * temp, 64, 64, 64));
                }
                break;
            case 2:
                temp = int(JumpFrame) % 3;
                if (y == 0) {
                    AnimX = 64 + 64 * temp;
                    AnimY = 128;
                    //sprite.setTextureRect(IntRect(64 + 64 * temp, 128, -64, 64));

                }
                else {
                    AnimX = 64 * temp;
                    AnimY = 128;
                    //sprite.setTextureRect(IntRect(64 * temp, 128, 64, 64));

                }
                break;
            }
        }
        void update(float time, float CurrentTime) {
            if (speed == 0) {
                AnimStatus = 0;
                RunFrame = 0;
                IdleFrame += time * 0.005;
            }
            else {
                AnimStatus = 1;
                IdleFrame = 0;
                RunFrame += time * 0.01;
            }
            SetAnim(AnimStatus, dir);
            switch (dir) {
            case 0:
                dx = dx * 0 - speed;
                break;
            case 1:
                dx = dy * 0 + speed;
                break;
            }
            positionX = positionX + dx * time;
            positionY = positionY + dy * time;

            speed = 0;
            sprite.setPosition(positionX, positionY);

            if (positionY < 273) {
                dy = dy + 0.00012;
            }
            else {
                dy = 0;
                isGrounded = true;
                canDoubleJump = false;
                jumpKeyPressed = false;
            }
            if (isGrounded == false) {
                JumpFrame += time * 0.0007;
                if (JumpFrame < 3) {
                    SetAnim(2, dir);
                }

            }
            else {
                JumpFrame = 0;
            }
            if (CurrentTime - LastAttack < 1) {
                float temp = (CurrentTime - LastAttack) * 24;
                if (temp < 6) {
                    if (dir == 0) {
                        //sprite.setTextureRect(IntRect(64 + 64 * int(temp), 256, -64, 64));
                        AnimX = 64 + 64 * int(temp);
                        AnimY = 256;
                    }
                    else {
                        //sprite.setTextureRect(IntRect(64 * int(temp), 256, 64, 64));
                        AnimX = 64 * int(temp);
                        AnimY = 256;
                    }
                }
            }
            DrawAnim();
        }
        void jump() {
            AnimStatus = 2;
            if (isGrounded || canDoubleJump) {
                dy = -0.30;
                if (isGrounded) {
                    isGrounded = false;
                    canDoubleJump = true;
                }
                else {
                    canDoubleJump = false;
                }
                jumpKeyPressed = true;
            }
        }
        void Attack(Hero x) {
            if (abs(x.positionY - positionY) < 81) {
                if (x.dir == 0) {
                    if (x.positionX - positionX < 81 && x.positionX - positionX > 0) {
                        health = health - 5;
                    }
                }
                else {
                    if (x.positionX - positionX > -81 && x.positionX - positionX < 0) {
                        health = health - 5;
                    }
                }
            }
        }
        void UpdateData(HeroUpdate zxc) {
            positionX = zxc.positionX;
            positionY = zxc.positionY;
            health = zxc.health;
            dir = zxc.dir;
            AnimX = zxc.AnimX;
            AnimY = zxc.AnimY;
        }
    };
    Hero p1 = { "16x16 knight 4 v3.png", 100, 100, 3 }, p2 = { "16x16 knight 3 v3.png", 400, 100, 3 };
    void SendMessageToClient(Hero& x, Hero& y) {
        while (true) {
            Sleep(10);
            HeroUpdate P1 = { x.positionX, x.positionY, x.health, x.dir, x.AnimX, x.AnimY };
            HeroUpdate P2 = { y.positionX, y.positionY, y.health, y.dir, y.AnimX, y.AnimY };
            char* buffer1 = new char[sizeof(HeroUpdate) + 1];
            char* buffer2 = new char[sizeof(HeroUpdate) + 1];

            buffer1[sizeof(HeroUpdate)] = '\0';
            memcpy(buffer1, &P1, sizeof(HeroUpdate));
            send(newConnection, buffer1, sizeof(HeroUpdate), NULL);

            buffer2[sizeof(HeroUpdate)] = '\0';
            memcpy(buffer2, &P2, sizeof(HeroUpdate));
            send(newConnection, buffer2, sizeof(HeroUpdate), NULL);

            delete[] buffer1;
            delete[] buffer2;
        }
    }
    void GetMessageFromClient() {
        while (true) {
            char msg[256];
            recv(newConnection, msg, sizeof(msg), NULL);
            p2movement = stoi(msg);
        }
    }
    void SentMessageToServer(int x) {
        char msg[256];
        sprintf(msg, "%d", x);
        send(newConnection, msg, sizeof(msg), NULL);
        Sleep(10);
    }
    void GetMessageFromSever() {
        while (true) {
            HeroUpdate P;
            char* buffer1 = new char[sizeof(HeroUpdate) + 1];
            char* buffer2 = new char[sizeof(HeroUpdate) + 1];

            buffer1[sizeof(HeroUpdate)] = '\0';
            recv(newConnection, buffer1, sizeof(HeroUpdate), NULL);
            memcpy(&P, buffer1, sizeof(HeroUpdate));
            p1.UpdateData(P);

            buffer2[sizeof(HeroUpdate)] = '\0';
            recv(newConnection, buffer2, sizeof(HeroUpdate), NULL);
            memcpy(&P, buffer2, sizeof(HeroUpdate));
            p2.UpdateData(P);

            delete[] buffer1;
            delete[] buffer2;
            Sleep(10);
        }
    }
    class Windows {
    public:
        Music music;
        SoundBuffer hitB, missB, jumpB;
        Sound hit, miss, jump;
        Texture BackgroundTexture, BlockTexture, EndTexture;
        Sprite BackGround, Block, EndScreen;
        sf::Font font;
        sf::Text ph1, ph2, end;
        Clock clock;
        Clock clock1;
        float CurrentTime = 0;
        RenderWindow window = { VideoMode(912, 513, 8), "Astifa" };
        Windows() {
            music.openFromFile("sound/music.mp3");
            music.setVolume(10.0);
            music.setLoop(true);
            
            hitB.loadFromFile("sound/hit.mp3");
            hit.setBuffer(hitB);
            hit.setVolume(10.0);

            jumpB.loadFromFile("sound/jump.mp3");
            jump.setBuffer(jumpB);
            jump.setVolume(10.0);
            font.loadFromFile("Intro.otf");
            window.setVisible(false);
            ph1 = { "", font, 20 };
            ph2 = { "", font, 20 };
            end = { "", font, 48 };
            BackgroundTexture.loadFromFile("images/BackGround.png");
            BlockTexture.loadFromFile("images/tileset1.png");
            EndTexture.loadFromFile("images/2.png");

            BackGround.setTexture(BackgroundTexture);
            Block.setTexture(BlockTexture);
            BackGround.setScale(0.5, 0.5);
            Block.setScale(9.12, 1);
            Block.setPosition(0, 413);
            EndScreen.setTexture(EndTexture);

            

            ph1.setPosition(30, 30);
            ph2.setPosition(850, 30);
            end.setFillColor(sf::Color::Black);
        }
        void WindowOpenOnServer(Hero& p1, Hero& p2) {
            music.play();
            while (window.isOpen()) {

                Event event;
                float time = clock.getElapsedTime().asMicroseconds();
                CurrentTime = clock1.getElapsedTime().asSeconds();
                clock.restart();
                time /= 400;

                while (window.pollEvent(event)) {
                    if (event.type == Event::Closed)
                        window.close();
                }

                if (Keyboard::isKeyPressed(Keyboard::A)) {
                    p1.dir = 0;
                    if (p1.positionX > -60) {
                        p1.speed = 0.2;
                    }
                    else {
                        p1.positionX = -60;
                    }
                }

                if (Keyboard::isKeyPressed(Keyboard::D)) {
                    p1.dir = 1;
                    if (p1.positionX < 783) {
                        p1.speed = 0.2;
                    }
                    else {
                        p1.positionX = 783;
                    }
                }

                if (Keyboard::isKeyPressed(Keyboard::W)) {
                    if (!p1.jumpKeyPressed) {
                        p1.jump();
                        jump.play();
                    }
                }
                else {
                    p1.jumpKeyPressed = false;
                }
                if (Keyboard::isKeyPressed(Keyboard::Space)) {
                    if (CurrentTime - p1.LastAttack >= 1) {
                        p2.Attack(p1);
                        p1.LastAttack = CurrentTime;
                        hit.play();
                    }
                }

                if (p2movement == 97) {
                    p2.dir = 0;
                    if (p2.positionX > -60) {
                        p2.speed = 0.2;
                    }
                    else {
                        p2.positionX = -60;
                    }
                }
                if (p2movement == 100) {
                    p2.dir = 1;
                    if (p2.positionX < 783) {
                        p2.speed = 0.2;
                    }
                    else {
                        p2.positionX = 783;
                    }
                }
                if (p2movement == 32) {
                    if (!p2.jumpKeyPressed && CurrentTime - p2jump > 0.1) {
                        p2.jump();
                        p2jump = CurrentTime;
                    }
                }
                else {
                    p2.jumpKeyPressed = false;
                }
                if (p2movement == 1) {
                    if (CurrentTime - p2.LastAttack >= 1) {
                        p1.Attack(p2);
                        p2.LastAttack = CurrentTime;
                        hit.play();
                    }
                }

                ph1.setString(to_string(p1.health));
                ph2.setString(to_string(p2.health));
                if (p1.health > 0 && p2.health > 0) {
                    //lock_guard<mutex> lock(p1_mutex);
                    p1.update(time, CurrentTime);
                    p2.update(time, CurrentTime);
                }
                window.clear();
                window.draw(BackGround);
                window.draw(Block);
                window.draw(p1.sprite);
                window.draw(p2.sprite);
                window.draw(ph1);
                window.draw(ph2);
                if (p1.health == 0) {
                    window.draw(EndScreen);
                    end.setString("Player 2 win!!!");
                    FloatRect TextSize = end.getLocalBounds();
                    end.setPosition((912 - TextSize.width) / 2, (513 - TextSize.height) / 2);
                    window.draw(end);
                }
                if (p2.health == 0) {
                    window.draw(EndScreen);
                    end.setString("Player 1 win!!!");
                    FloatRect TextSize = end.getLocalBounds();
                    end.setPosition((912 - TextSize.width) / 2, (513 - TextSize.height) / 2);
                    window.draw(end);
                }
                window.display();
            }
        }
        void WindowOpenOnClient(Hero& p1, Hero& p2) {
            thread th1(GetMessageFromSever);
            th1.detach();
            int P1LastAtack = 0;
            music.play();
            while (window.isOpen()) {
                Event event;
                int message = 0;
                if (Keyboard::isKeyPressed(Keyboard::Left)) {
                    message = 97;
                }
                if (Keyboard::isKeyPressed(Keyboard::Right)) {
                    message = 100;
                }
                if (Keyboard::isKeyPressed(Keyboard::Up)) {
                    SentMessageToServer(32);
                    jump.play();
                }
                if (Keyboard::isKeyPressed(Keyboard::Enter)) {
                    SentMessageToServer(1);
                    hit.play();
                }
                if (p1.LastAttack != P1LastAtack) {
                    hit.play();
                    P1LastAtack = p1.LastAttack;
                }
                SentMessageToServer(message);
                ph1.setString(to_string(p1.health));
                ph2.setString(to_string(p2.health));
                window.clear();
                window.draw(BackGround);
                window.draw(Block);
                window.draw(p1.sprite);
                window.draw(p2.sprite);
                window.draw(ph1);
                window.draw(ph2);
                if (p1.health > 0 && p2.health > 0) {
                    p1.DrawAnim();
                    p2.DrawAnim();
                    p1.sprite.setPosition(p1.positionX, p1.positionY);
                    p2.sprite.setPosition(p2.positionX, p2.positionY);
                }
                if (p1.health == 0) {
                    window.draw(EndScreen);
                    end.setString("Player 2 win!!!");
                    FloatRect TextSize = end.getLocalBounds();
                    end.setPosition((912 - TextSize.width) / 2, (513 - TextSize.height) / 2);
                    window.draw(end);
                }
                if (p2.health == 0) {
                    window.draw(EndScreen);
                    end.setString("Player 1 win!!!");
                    FloatRect TextSize = end.getLocalBounds();
                    end.setPosition((912 - TextSize.width) / 2, (513 - TextSize.height) / 2);
                    window.draw(end);
                }
                window.display();
            }
        }
    };
    Windows windowa;
	public ref class GUI : public System::Windows::Forms::Form
	{
	public:
		GUI(void)
		{
			InitializeComponent();
		}

	protected:
		~GUI()
		{
			if (components)
			{
				delete components;
			}
		}

    private: System::Windows::Forms::Button^ button1;
    private: System::Windows::Forms::Label^ label1;
    private: System::Windows::Forms::Button^ button2;
    private: System::Windows::Forms::TextBox^ textBox1;
    private: System::Windows::Forms::Label^ label2;
	protected:

	private:
		System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code
		void InitializeComponent(void)
		{
            System::ComponentModel::ComponentResourceManager^ resources = (gcnew System::ComponentModel::ComponentResourceManager(GUI::typeid));
            this->button1 = (gcnew System::Windows::Forms::Button());
            this->label1 = (gcnew System::Windows::Forms::Label());
            this->button2 = (gcnew System::Windows::Forms::Button());
            this->textBox1 = (gcnew System::Windows::Forms::TextBox());
            this->label2 = (gcnew System::Windows::Forms::Label());
            this->SuspendLayout();
            // 
            // button1
            // 
            this->button1->BackColor = System::Drawing::Color::RosyBrown;
            this->button1->FlatStyle = System::Windows::Forms::FlatStyle::Popup;
            this->button1->Font = (gcnew System::Drawing::Font(L"Kristen ITC", 15.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(0)));
            this->button1->Location = System::Drawing::Point(117, 188);
            this->button1->Name = L"button1";
            this->button1->Size = System::Drawing::Size(200, 72);
            this->button1->TabIndex = 0;
            this->button1->Text = L"Player 1";
            this->button1->UseVisualStyleBackColor = false;
            this->button1->Click += gcnew System::EventHandler(this, &GUI::button1_Click);
            // 
            // label1
            // 
            this->label1->AutoSize = true;
            this->label1->BackColor = System::Drawing::Color::Transparent;
            this->label1->Font = (gcnew System::Drawing::Font(L"Kristen ITC", 20.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(0)));
            this->label1->ForeColor = System::Drawing::Color::Orange;
            this->label1->Location = System::Drawing::Point(272, 149);
            this->label1->Name = L"label1";
            this->label1->Size = System::Drawing::Size(223, 36);
            this->label1->TabIndex = 1;
            this->label1->Text = L"Choose player:";
            // 
            // button2
            // 
            this->button2->BackColor = System::Drawing::Color::RosyBrown;
            this->button2->FlatStyle = System::Windows::Forms::FlatStyle::Popup;
            this->button2->Font = (gcnew System::Drawing::Font(L"Kristen ITC", 15.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(0)));
            this->button2->Location = System::Drawing::Point(461, 188);
            this->button2->Name = L"button2";
            this->button2->Size = System::Drawing::Size(200, 72);
            this->button2->TabIndex = 2;
            this->button2->Text = L"Player 2";
            this->button2->UseVisualStyleBackColor = false;
            this->button2->Click += gcnew System::EventHandler(this, &GUI::button2_Click);
            // 
            // textBox1
            // 
            this->textBox1->BackColor = System::Drawing::Color::DarkViolet;
            this->textBox1->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
            this->textBox1->Font = (gcnew System::Drawing::Font(L"Kristen ITC", 12, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(0)));
            this->textBox1->Location = System::Drawing::Point(241, 355);
            this->textBox1->Name = L"textBox1";
            this->textBox1->Size = System::Drawing::Size(0, 0);
            this->textBox1->TabIndex = 3;
            this->textBox1->Text = L"000.000.000.000";
            // 
            // label2
            // 
            this->label2->AutoSize = true;
            this->label2->BackColor = System::Drawing::Color::Transparent;
            this->label2->Font = (gcnew System::Drawing::Font(L"Kristen ITC", 15.75F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(0)));
            this->label2->ForeColor = System::Drawing::Color::Orange;
            this->label2->Location = System::Drawing::Point(190, 355);
            this->label2->Name = L"label2";
            this->label2->Size = System::Drawing::Size(45, 29);
            this->label2->TabIndex = 5;
            this->label2->Text = L"";
            // 
            // GUI
            // 
            this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
            this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
            this->BackgroundImage = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"$this.BackgroundImage")));
            this->BackgroundImageLayout = System::Windows::Forms::ImageLayout::Stretch;
            this->CausesValidation = false;
            this->ClientSize = System::Drawing::Size(784, 411);
            this->Controls->Add(this->label2);
            this->Controls->Add(this->textBox1);
            this->Controls->Add(this->button2);
            this->Controls->Add(this->label1);
            this->Controls->Add(this->button1);
            this->Cursor = System::Windows::Forms::Cursors::Default;
            this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::None;
            this->MaximizeBox = false;
            this->MinimizeBox = false;
            this->Name = L"GUI";
            this->StartPosition = System::Windows::Forms::FormStartPosition::CenterScreen;
            this->Text = L"GUI";
            this->TransparencyKey = System::Drawing::Color::Transparent;
            this->ResumeLayout(false);
            this->PerformLayout();

		}
#pragma endregion
	private: System::Void button1_Click(System::Object^ sender, System::EventArgs^ e) {
        WSAData wsaDATA;
        WORD DLLVersion = MAKEWORD(2, 1);
        if (WSAStartup(DLLVersion, &wsaDATA) != 0) {
            cout << "error";
            exit(1);
        }
        SOCKADDR_IN addr;
        int sizeofaddr = sizeof(addr);
        addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        addr.sin_port = htons(1111);
        addr.sin_family = AF_INET;

        SOCKET sListen = socket(AF_INET, SOCK_STREAM, NULL);
        bind(sListen, (SOCKADDR*)&addr, sizeof(addr));
        listen(sListen, SOMAXCONN);

        newConnection = accept(sListen, (SOCKADDR*)&addr, &sizeofaddr);
        if (newConnection == 0) {
            cout << "error";
        }
        else {
            this->Close();
            windowa.window.setVisible(true);
            thread th1(SendMessageToClient, ref(p1), ref(p2));
            thread th2(GetMessageFromClient);
            windowa.WindowOpenOnServer(p1, p2);
            th1.join();
            th2.join();
        }
	}
    private: System::Void button2_Click(System::Object^ sender, System::EventArgs^ e) {
        this -> Close();
        windowa.window.setVisible(true);
        WSAData wsaDATA;
        WORD DLLVersion = MAKEWORD(2, 1);
        if (WSAStartup(DLLVersion, &wsaDATA) != 0) {
            cout << "error";
            exit(1);
        }
        SOCKADDR_IN addr;
        int sizeofaddr = sizeof(addr);
        addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        addr.sin_port = htons(1111);
        addr.sin_family = AF_INET;

        newConnection = socket(AF_INET, SOCK_STREAM, NULL);
        connect(newConnection, (SOCKADDR*)&addr, sizeof(addr));
        if (newConnection == 0) {
            cout << "error";
        }
        else {
            windowa.WindowOpenOnClient(p1, p2);
        }
	}
};
}