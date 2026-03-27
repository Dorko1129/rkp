enum eVertexArrayObject {
	VAOVerticesData,
	VAOCount
};
enum eBufferObject {
	VBOVerticesData,
	BOCount
};
enum eProgram {
	QuadScreenProgram,
	ProgramCount
};
enum eTexture {
	NoTexture,		// fixes 0 sized array problem
	TextureCount
};

#include <common.cpp>

GLchar				windowTitle[] = "Drag-and-Drop";
GLfloat				aspectRatio;
/* -1 jelentése, hogy nem vonszolunk semmit. */
/* -1 means we are not dragging. */
GLint				dragged = -1;
/* Vektor a szakasz végpontjainak tárolásához. */
/* Vector for storing end points of a line. */
static vector<vec2>	verticesData = {
	vec2(-0.5f, -0.5f),
	vec2( 0.5f,  0.5f),
};

void initShaderProgram() {
	ShaderInfo shader_info[ProgramCount][3] = { {
		{ GL_VERTEX_SHADER,		"./vertexShader.glsl" },
		{ GL_FRAGMENT_SHADER,	"./fragmentShader.glsl" },
		{ GL_NONE, nullptr } } };

	for (int programItem = 0; programItem < ProgramCount; programItem++) {
		program[programItem] = LoadShaders(shader_info[programItem]);
		/** Shader változó location lekérdezése. */
		/** Getting shader variable location. */
		locationMatModel = glGetUniformLocation(program[programItem], "matModel");
		locationMatView = glGetUniformLocation(program[programItem], "matView");
		locationMatProjection = glGetUniformLocation(program[programItem], "matProjection");
	}
	/** Csatoljuk a vertex array objektumunkat a paraméterhez. */
	/** glBindVertexArray binds the vertex array object to the parameter. */
	glBindVertexArray(VAO[VAOVerticesData]);
	/** A GL_ARRAY_BUFFER nevesített csatolóponthoz kapcsoljuk a vertex buffert (ide kerülnek a csúcspont adatok). */
	/** We attach the vertex buffer to the GL_ARRAY_BUFFER node (vertices are stored here). */
	glBindBuffer(GL_ARRAY_BUFFER, BO[VBOVerticesData]);
	/** Másoljuk az adatokat a bufferbe! Megadjuk az aktuálisan csatolt buffert, azt hogy hány byte adatot másolunk,
		a másolandó adatot, majd a feldolgozás módját is meghatározzuk: most az adat nem változik a feltöltés után. */
	/** Copy the data to the buffer! First parameter is the currently attached buffer, second is the size of the buffer to be copied,
		third is the array of data, fourth is working mode: now the data can not be modified after this step. */
	glBufferData(GL_ARRAY_BUFFER, verticesData.size() * sizeof(vec2), verticesData.data(), GL_STATIC_DRAW);
	/** Engedélyezzük az imént definiált location = 0 attribútumot (vertexShader.glsl ). */
	/** Enable the previously defined location = 0 attributum (vertexShader.glsl ). */
	glEnableVertexAttribArray(0);
	/** Ezen adatok szolgálják a location = 0 vertex attribútumot (itt: pozíció).
		Elsõként megadjuk ezt az azonosítószámot (vertexShader.glsl).
		Utána az attribútum méretét (vec3, láttuk a shaderben).
		Harmadik az adat típusa.
		Negyedik az adat normalizálása, ez maradhat FALSE jelen példában.
		Az attribútum értékek hogyan következnek egymás után? Milyen lépésköz után találom a következõ vertex adatait?
		Végül megadom azt, hogy honnan kezdõdnek az értékek a pufferben. Most rögtön, a legelejétõl veszem õket. */
	/** These values are for location = 0 vertex attribute (position).
		First is the location (vertexShader.glsl).
		Second is attribute size (vec3, as in the shader).
		Third is the data type.
		Fourth defines whether data shall be normalized or not, this is FALSE for the examples of the course.
		Fifth is the distance in bytes to the next vertex element of the array.
		Last is the offset of the first vertex data of the buffer. Now it is the start of the array. */
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vec2), 0);
	/** Az aktuális példaprogram bekapcsolása. */
	/** Switch on the current sample program. */
	glUseProgram(program[QuadScreenProgram]);
	/** Mátrixok kezdőértékének beállítása. */
	/** Set the matrices to the initial values. */
	matModel = mat4(1.0);
	matView = lookAt(
		vec3(0.0f, 0.0f, 9.0f),		// the position of your camera, in world space
		vec3(0.0f, 0.0f, 0.0f),		// where you want to look at, in world space
		vec3(0.0f, 1.0f, 0.0f));	// upVector, probably glm::vec3(0,1,0), but (0,-1,0) would make you looking upside-down, which can be great too
	glUniformMatrix4fv(locationMatModel, 1, GL_FALSE, value_ptr(matModel));
	glUniformMatrix4fv(locationMatView, 1, GL_FALSE, value_ptr(matView));
	glUniformMatrix4fv(locationMatProjection, 1, GL_FALSE, value_ptr(matProjection));
}

GLfloat distanceSquare(vec2 p1, vec2 p2) {
	vec2		delta = p1 - p2;	// delta.xy = p1.xy - p2.xy
	/** delta.x és delta.y segítségével ki tudjuk számolni p1 és p2 pontok távolságának négyzetét. */
	/** From delta.x and delta.y we can calculate the square of the distance of p1 and p2 points. */
	return dot(delta, delta);		// delta.x * delta.x + delta.y * delta.y
}
/** mousePos az egér pozíciója, amit paraméterként kapunk. */
/** mousePos is the mouse position, arriving as vec2. */
GLint getActivePoint(vector<vec2> p, GLfloat sensitivity, vec2 mousePosition) {
	/** Ha az s a sensitivity négyzete, akkor a távolság számításnál és hasonlításnál megtakaríthatunk egy négyzetgyökvonást. Ez jelentõs gyorsítás. */
	/** If s is square of the sensitivity, then we can spare a square root computation at the distance calculations and comparisons. This is a significant speedup. */
	GLfloat		sensitivitySquare	= sensitivity * sensitivity;
	/** Az egyes p pontok távolság négyzetének vizsgálata a mousePosition pozíciójához. */
	/** Checking the square of the distanse of each p to mousePosition. */
	for (GLint i = 0; i < p.size(); i++)
		if (distanceSquare(p[i], mousePosition) < sensitivitySquare)
			/** Ha p közelebb van a sensitivity-ben megadott értéknél, akkor visszatérünk vele, mint megragadott ponttal akkor is, ha van nála közelebbi a vektorban hátrébb. */
			/** If p is closer than the sensitivity, then it is returned as a grabbed point even if there is closer later in the vector of points. */
			return i;
	/** Semmi sincs közel az egérhez, -1 jelentése, hogy semmit nem fogtunk meg és vonszolunk magunkkal. */
	/** Nothing is close to the mouse, -1 means that nothing is grabbed and dragged. */
	return -1;
}

void display(GLFWwindow* window, double currentTime) {
	/* Töröljük le a kiválasztott buffereket! Fontos lehet minden egyes alkalommal törölni! */
	/* Let's clear the selected buffers! Usually importand to clear it each time! */
	glClear(GL_COLOR_BUFFER_BIT);
	/** A megadott adatok segítségével szakaszt és annak végpontjait rajzoljuk meg. */
	/** We draw a line and its endpoints with the defined array. */
	glDrawArrays(GL_LINES, 0, verticesData.size());
	glDrawArrays(GL_POINTS, 0, verticesData.size());
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
	/** A minimalizálás nem fog fagyni a minimum 1 értékkel. */
	/** Minimize will not freeze with minimum value 1. */
	windowWidth = glm::max(width, 1);
	windowHeight = glm::max(height, 1);

	aspectRatio = (float)windowWidth / (float)windowHeight;
	/** A kezelt képernyõ beállítása a teljes (0, 0, width, height) területre. */
	/** Set the viewport for the full (0, 0, width, height) area. */
	glViewport(0, 0, windowWidth, windowHeight);
	/** Orthographic projekció beállítása, worldSize lesz a szélesség és magasság közül a kisebbik. */
	/** Set up orthographic projection, worldSize will equal the smaller value of width or height. */
	if (projectionType == Orthographic)
		if (windowWidth < windowHeight)
			matProjection = ortho(-worldSize, worldSize, -worldSize / aspectRatio, worldSize / aspectRatio, -100.0, 100.0);
		else
			matProjection = ortho(-worldSize * aspectRatio, worldSize * aspectRatio, -worldSize, worldSize, -100.0, 100.0);
	else
		matProjection = perspective(
			radians(45.0f),	// The vertical Field of View, in radians: the amount of "zoom". Think "camera lens". Usually between 90° (extra wide) and 30° (quite zoomed in)
			aspectRatio,	// Aspect Ratio. Depends on the size of your window. Notice that 4/3 == 800/600 == 1280/960, sounds familiar?
			0.1f,			// Near clipping plane. Keep as big as possible, or you'll get precision issues.
			100.0f			// Far clipping plane. Keep as little as possible.
		);
	/** Uniform változók beállítása. */
	/** Setup uniform variables. */
	glUniformMatrix4fv(locationMatProjection, 1, GL_FALSE, value_ptr(matProjection));
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	/** ESC billentyûre kilépés. */
	/** Exit on ESC key. */
	if ((action == GLFW_PRESS) && (key == GLFW_KEY_ESCAPE)) glfwSetWindowShouldClose(window, GLFW_TRUE);
	/** A billentyûk lenyomásának és felengedésének regisztrálása. Lehetõvé teszi gombkombinációk használatát. */
	/** Let's register press and release events for keys. Enables the usage of key combinations. */
	if (action == GLFW_PRESS)
		keyboard[key] = GL_TRUE;
	else if (action == GLFW_RELEASE)
		keyboard[key] = GL_FALSE;
	/** Az O billentyû lenyomása az orthographic leképezést kapcsolja be. */
	/** Pressing O induces orthographic projection. */
	if (key == GLFW_KEY_O && action == GLFW_PRESS) {
		projectionType = Orthographic;
		framebufferSizeCallback(window, windowWidth, windowHeight);
	}
	/** A P billentyû lenyomása a perspective leképezést kapcsolja be. */
	/** Pressing P induces perspective projection. */
	if (key == GLFW_KEY_P && action == GLFW_PRESS) {
		projectionType = Perspective;
		framebufferSizeCallback(window, windowWidth, windowHeight);
	}
}
/** Az egér mutató helyét kezelõ függvény. */
/** Callback function for mouse position change. */
void cursorPosCallback(GLFWwindow* window, double xPos, double yPos) {
	/** Ha vonszolunk egy pontot, akkor számoljuk ki a normalizált koordinátáit. */
	/** If we are dragging a point, let's calculate the normalized values. */
	if (dragged >= 0) {
		dvec2	mousePosition;
		/** Kiírjuk az egérkurzor pixelkoordinátáit. */
		/** Display the pixel coordinates of the mouse. */
		cout << "cursorPosCallback\t\t\t" << xPos << "\t" << yPos << endl;
		/** A pixelkoordináták [-1..+1] intervallumra transzformált értékei. */
		/** The pixelcoordinates transformed to the [-1..+1] interval values. */
		mousePosition.x = xPos * 2.0f / (GLdouble)windowWidth - 1.0f;
		mousePosition.y = ((GLdouble)windowHeight - yPos) * 2.0f / (GLdouble)windowHeight - 1.0f;
		if (windowWidth < windowHeight)
			mousePosition.y /= aspectRatio;
		else
			mousePosition.x *= aspectRatio;
		cout << "cursorPosCallback normalized coords\t" << mousePosition.x << "\t" << mousePosition.y << endl;
		/** Tároljuk el a módosított értékeket. */
		/** Let's store the modified values. */
		verticesData.at(dragged) = mousePosition;
		verticesData[dragged] = mousePosition;
		/** Mozgassuk a módosított értékeket a GPU memóriájába. */
		/** Let's transfer the modified values to the GPU. */
		glBindBuffer(GL_ARRAY_BUFFER, BO[VBOVerticesData]);
		glBufferData(GL_ARRAY_BUFFER, verticesData.size() * sizeof(vec2), verticesData.data(), GL_STATIC_DRAW);
	}
}
/** Az egér gombjaihoz köthetõ események kezelése. */
/** Callback function for mouse button events. */
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
	/** Az egér bal gombjának megnyomása indíthat el vonszolást. */
	/** Pressing left mouse button might initiate dragging. */
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		dvec2	mousePosition;
		/** Kinyerjük az egérkurzor pixelkoordinátáit. */
		/** Obtain the pixel coordinates of the mouse. */
		glfwGetCursorPos(window, &mousePosition.x, &mousePosition.y);
		cout << "mouseButtonCallback glfwGetCursorPos\t" << mousePosition.x << "\t" << mousePosition.y << endl;
		/** A pixelkoordináták [-1..+1] intervallumra transzformált értékei. */
		/** The pixelcoordinates transformed to the [-1..+1] interval values. */
		mousePosition.x = mousePosition.x * 2.0f / (GLdouble)windowWidth - 1.0f;
		mousePosition.y = ((GLdouble)windowHeight - mousePosition.y) * 2.0f / (GLdouble)windowHeight - 1.0f;
		if (windowWidth < windowHeight)
			mousePosition.y /= aspectRatio;
		else
			mousePosition.x *= aspectRatio;
		cout << "mouseButtonCallback normalized coords\t" << mousePosition.x << "\t" << mousePosition.y << endl;
		/** dragged lesz az indexe a kiválasztott pontnak [0, 1, ...], -1 jelentése, hogy semmit nem fogtunk meg. */
		/** dragged is the index of the point that is selected [0, 1, ...], -1 means nothing is grabbed. */
		dragged = getActivePoint(verticesData, 0.1f, mousePosition);
	}
	/** Az egér bal gombjának felengedése mindenképp megszünteti a vonszolási üzemmódot. */
	/** Releasing left mouse button stops dragging operation, even if it was not active before. */
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
		dragged = -1;
}

int main(void) {
	/** Az alkalmazáshoz kapcsolódó elõkészítõ lépések (paraméterek: major, minor OpenGL verziók, OpenGL pofile). */
	/** The first initialization steps of the program (params: major, minor OpenGL versions, OpenGL pofile). */
	init(3, 3, GLFW_OPENGL_COMPAT_PROFILE);
	/** A shader programok betöltése. */
	/** Loading the shader programs for rendering. */
	initShaderProgram();
	/** A window elkészítése után a képernyő beállítása. */
	/** Setup screen after window creation. */
	framebufferSizeCallback(window, windowWidth, windowHeight);
	/** Karakterkódolás a szövegekhez. */
	/** Setting locale for characters of texts. */
	setlocale(LC_ALL, "");
	cout << "Drag-and-Drop using mouse" << endl << endl;
	cout << "Keyboard control" << endl;
	cout << "ESC\t\texit" << endl;
	cout << "O\t\tinduces orthographic projection" << endl;
	cout << "P\t\tinduces perspective projection" << endl << endl;
	cout << "Weekly tasks" << endl;
	cout << "Gyakorlat 1: Színezzük kékre a vonalat!" << endl;
	cout << "Gyakorlat 2: Tegyük működőképessé perspektíva mellett is a vonszolást!" << endl;
	cout << "Gyakorlat 3: Ctrl gomb lenyomására az összes pontot mozgassuk!" << endl;
	cout << "Activity  1: Color the line to blue!" << endl;
	cout << "Activity  2: Make drag-and-drop working for perspective projection!" << endl;
	cout << "Activity  3: Let pressing Ctrl to drag all the points!" << endl;
	/** A megadott window struktúra "close flag" vizsgálata. */
	/** Checks the "close flag" of the specified window. */
	while (!glfwWindowShouldClose(window)) {
		/** A kód, amellyel rajzolni tudunk a GLFWwindow objektumunkba. */
		/** Call display function which will draw into the GLFWwindow object. */
		display(window, glfwGetTime());
		/** Double buffered mûködés. */
		/** Double buffered working = swap the front and back buffer here. */
		glfwSwapBuffers(window);
		/** Események kezelése az ablakunkkal kapcsolatban, pl. gombnyomás. */
		/** Handle events related to our window, e.g.: pressing a key or moving the mouse. */
		glfwPollEvents();
	}
	/** Felesleges objektumok törlése. */
	/** Clenup the unnecessary objects. */
	cleanUpScene(EXIT_SUCCESS);
	/** Kilépés EXIT_SUCCESS kóddal. */
	/** Stop the software and exit with EXIT_SUCCESS code. */
	return EXIT_SUCCESS;
}