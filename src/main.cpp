//Sully Chen ©2017
#include <iostream>
#include <vector>
#include <math.h>
#include <fstream>
#include <unistd.h>
#include "button.hpp"
#include "node.hpp"


using namespace std;

//resolution of the window
const int width = 1280;
const int height = 800;

//font for drawing
sf::Font font;

//Vectors of objects
vector<Button> buttons;
vector<Node> nodes;
vector<Node*> node_chain;

int numInputs = 0; //keeps track of the total number of input nodes

sf::RenderWindow* window;

void Update(); //Update loop
void Draw(); //Draw loop
void CreateFlatInput(); //Creates a flat input
void CreateImageInput(); //Creates an image input
void CreateFCL(); //Creates a fully connected later
void CreateConv(); //Creates a convolutional layer
void CreateActivation(); //Creates an activation
void LinkNodes(); //Links two nodes together
void Train(); //Trains network
void RemoveNode(Node* n); //Removes a node from the graph
void ParseModelString(string);
string Prompt(sf::String message);
string RecurseNode(Node* n, string name); //Used to process graph
string GenerateModelCode(); //Generates the code for the Keras Model

//Update loop
void Update()
{
    for (int i = 0; i < buttons.size(); i++)
        buttons[i].Update(sf::Mouse::getPosition(*window), sf::Mouse::isButtonPressed(sf::Mouse::Left));
    for (int i = 0; i < nodes.size(); i++)
    {
        Node* n = &nodes[i];
        //if a node is clicked on, it will be added to the chain
        if (n->Update(sf::Mouse::getPosition(*window), sf::Mouse::isButtonPressed(sf::Mouse::Left)))
        {
            //check if the node is already added to the chain, in which case it will be removed instead
            bool node_present = false;
            for (int j = 0; j < node_chain.size(); j++)
                if (node_chain[j] == n)
                {
                    node_chain.erase(node_chain.begin() + j);
                    node_present = true;
                    break;
                }

            //if the node isn't in the chain, add it
            if (!node_present)
            {
                //if there are already 2 nodes in the chain, remove them all and add the new one
                if (node_chain.size() == 2)
                    node_chain.clear();
                node_chain.push_back(n);
            }
        }
    }
}

//Draw loop
void Draw()
{
    //clear window
    window->clear(sf::Color::White);

    //draw buttons
    for (int i = 0; i < buttons.size(); i++)
        buttons[i].Draw(window);

    //highlight selected nodes
    for (int i = 0; i < node_chain.size(); i++)
    {
        sf::Color color;
        if (i % 2)
            color = sf::Color::Blue;
        else
            color = sf::Color::Red;

        //get bounding box
        sf::Rect<float> rect = node_chain[i]->Message.getGlobalBounds();
        sf::RectangleShape rectangle;
        rectangle.setOutlineColor(color);
        rectangle.setOutlineThickness(5);
        rectangle.setPosition(rect.left - 30, rect.top - 15);
        rectangle.setSize(sf::Vector2f(rect.width + 60, rect.height + 30));
        window->draw(rectangle);
    }

    //Draw nodes
    for (int i = 0; i < nodes.size(); i++)
        nodes[i].Draw(window);

    //Draw links
    for (int i = 0; i < nodes.size(); i++)
    {
        Node n = nodes[i];
        for (int j = 0; j < n.Children.size(); j++)
        {
            Node* c = n.Children[j];
            sf::Rect<float> parentrect = n.Message.getGlobalBounds();
            sf::Rect<float> childrect = c->Message.getGlobalBounds();
            parentrect.width += 50;
            parentrect.height += 20;
            parentrect.left -= 25;
            parentrect.top -= 10;
            childrect.width += 50;
            childrect.height += 20;
            childrect.left -= 25;
            childrect.top -= 10;
            float deltaX = childrect.left - parentrect.left;
            float deltaY = childrect.top - parentrect.top;
            float dx = deltaX / sqrt(deltaX*deltaX + deltaY*deltaY);
            float dy = deltaY / sqrt(deltaX*deltaX + deltaY*deltaY);
            sf::Vector2f startpoint(parentrect.left + parentrect.width / 2 + dx * parentrect.width / 2,
                                    parentrect.top + parentrect.height / 2 + dy * parentrect.height / 2);
            sf::Vector2f endpoint(childrect.left + childrect.width / 2 - dx * childrect.width / 2,
                                  childrect.top + childrect.height / 2 - dy * childrect.height / 2);
            sf::VertexArray line(sf::LineStrip, 2);
            line[0].position = startpoint;
            line[0].color = sf::Color::Green;
            line[1].position = endpoint;
            line[1].color = sf::Color::Red;
            window->draw(line);
        }
    }
}

string Prompt(sf::String message)
{

    sf::Text text;
    text.setFillColor(sf::Color::Black);
    text.setString(message);
    text.setFont(font);
    text.setOutlineThickness(1);
    text.setStyle(sf::Text::Regular);
    text.setPosition(20, window->getSize().y - text.getGlobalBounds().height);
    text.setCharacterSize(22);

    sf::String response = "";

    bool exit = false;
    while (!exit)
    {
        //check for events
        sf::Event event;
        while (window->pollEvent(event))
        {
            //close if close signal detected
            if (event.type == sf::Event::Closed)
                window->close();

            if(event.type == sf::Event::TextEntered)
            {
                if (event.text.unicode < 128 && event.text.unicode > 28)
                    response += event.text.unicode;
            }

            else if(event.type == sf::Event::KeyPressed)
            {
                if(event.key.code == sf::Keyboard::BackSpace && response.getSize() > 0) // delete the last character
                    response.erase(response.getSize() - 1);
                if(event.key.code == sf::Keyboard::Return) // delete the last character
                    exit = true;
            }
        }

        text.setString(message + response);

        Draw();
        window->draw(text);
        window->display();
    }

    return response.toAnsiString();
}

void CreateFlatInput()
{
    node_chain.clear();
    string name = "Node " + to_string(nodes.size());
    int inputDim = 0;
    inputDim = atoi(Prompt("Number of inputs?: ").c_str());
    Node n(sf::Vector2f(500, 50), "Input " + sf::String(to_string(inputDim)),
           &font, 18, sf::Color::Black);
    n.Type = "flat";
    n.Name = "x" + to_string(numInputs);
    n.Command = n.Name + " = Input(shape=(" + to_string(inputDim) + ",))\n";
    nodes.push_back(n);
    numInputs++;
}

void CreateImageInput()
{
    node_chain.clear();
    string name = "Node " + to_string(nodes.size());
    int numLayers = 0;
    int dimX = 0;
    int dimY = 0;
    numLayers = atoi(Prompt("Number of layers?: ").c_str());
    dimX = atoi(Prompt("Image width?: ").c_str());
    dimY = atoi(Prompt("Image height?: ").c_str());
    Node n(sf::Vector2f(500, 50), sf::String("Input " + to_string(numLayers)) + "x"
           + sf::String(to_string(dimX) + "x" + to_string(dimY)),
           &font, 18, sf::Color::Black);
    n.Type = "image";
    n.Name = "x" + to_string(numInputs);
    n.Command = n.Name + " = Input(shape=(" + to_string(numLayers) + ", " +
    to_string(dimX) + ", " + to_string(dimY) + "))\n";

    /*//reshape flat input
     n.Command += n.Name + " = Reshape((" + to_string(numLayers) +
     ", " + to_string(dimX) + ", " + to_string(dimY) + "), "
     "input_shape=(" + to_string(numLayers*dimX*dimY) +
     ",))("+ n.Name + ")\n";*/
    nodes.push_back(n);
    numInputs++;
}

void CreateFCL()
{
    node_chain.clear();
    string name = "Node " + to_string(nodes.size());
    int size = 0;
    size = atoi(Prompt("Number of neurons?: ").c_str());
    Node n(sf::Vector2f(500, 50), sf::String(to_string(size)) + "nn",
           &font, 18, sf::Color::Black);
    n.Type = "flat";
    n.Name = "x" + to_string(nodes.size());
    n.Command = n.Name + " = Dense(" + to_string(size) + ")";
    nodes.push_back(n);
}

void CreateActivation()
{
    node_chain.clear();
    string name = "Node " + to_string(nodes.size());
    string activation = "";
    activation = Prompt("What kind of activation? (sigmoid, softmax, tanh, relu): ");
    while (true)
    {
        if (activation == "sigmoid")
            break;
        if (activation == "softmax")
            break;
        if (activation == "tanh")
            break;
        if (activation == "relu")
            break;
        activation = Prompt("Invalid activation! What kind of activation? (sigmoid, softmax, tanh, relu): ");
    }
    Node n(sf::Vector2f(500, 50), sf::String(activation), &font, 18, sf::Color::Black);
    n.Type = "activation";
    n.Name = "x" + to_string(nodes.size());
    n.Command = n.Name + " = Activation(\"" + activation + "\")";
    nodes.push_back(n);
}

void CreateConv()
{
    node_chain.clear();
    string name = "Node " + to_string(nodes.size());
    int numFilters = 0;
    int kerneldimX = 0;
    int kerneldimY = 0;
    int strideX = 0;
    int strideY = 0;
    numFilters = atoi(Prompt("Number of filters?: ").c_str());
    kerneldimX = atoi(Prompt("Kernel width?: ").c_str());
    kerneldimY = atoi(Prompt("Kernel height?: ").c_str());
    strideX = atoi(Prompt("Stride width?: ").c_str());
    strideY = atoi(Prompt("Stride height?: ").c_str());
    Node n(sf::Vector2f(500, 50), sf::String(to_string(numFilters)) + "x"
           + sf::String(to_string(kerneldimX) + "x" + to_string(kerneldimY)
                        + "conv " + to_string(strideX) + "x" + to_string(strideY) + "stride"),
           &font, 18, sf::Color::Black);
    n.Type = "image";
    n.Name = "x" + to_string(nodes.size());
    n.Command = n.Name + " = Conv2D(" + to_string(numFilters) + ", ("
    + to_string(kerneldimX) + ", " + to_string(kerneldimY) + ")"
    + ", strides=(" + to_string(strideX) + "," + to_string(strideY) + "))";
    nodes.push_back(n);
}

void LinkNodes()
{
    if (node_chain.size() == 2)
    {
        //check if the child node is already added, and remove it if so
        bool node_present = false;
        for (int i = 0; i < node_chain[0]->Children.size(); i++)
            if (node_chain[0]->Children[i] == node_chain[1])
            {
                node_present = true;
                node_chain[0]->Children.erase(node_chain[0]->Children.begin() + i);
            }


        //if not, add it
        if (!node_present)
            node_chain[0]->Children.push_back(node_chain[1]);

        //check if the parent node is already added, and remove it if so
        node_present = false;
        for (int i = 0; i < node_chain[1]->Parents.size(); i++)
            if (node_chain[1]->Parents[i] == node_chain[0])
            {
                node_present = true;
                node_chain[1]->Parents.erase(node_chain[1]->Parents.begin() + i);
            }

        //if not, add it
        if (!node_present)
        {
            node_chain[1]->Parents.push_back(node_chain[0]);
            if (node_chain[0]->Type == "image" && node_chain[1]->Type == "activation")
                node_chain[1]->Type = "image";
        }

        //cout << node_chain[0]->Children.size() << node_chain[1]->Children.size()
        //   << node_chain[0]->Parents.size() << node_chain[1]->Parents.size() << endl;
    }
    node_chain.clear();
}

void RemoveNode(Node* n)
{
    //remove the node from the parents' list of children
    for (int i = 0; i < n->Parents.size(); i++)
        for (int j = 0; j < n->Parents[i]->Children.size(); j++)
            if (n == n->Parents[i]->Children[j])
                n->Parents[i]->Children.erase(n->Parents[i]->Children.begin() + j);

    //remove the node from the childrens' list of parents
    for (int i = 0; i < n->Children.size(); i++)
        for (int j = 0; j < n->Children[i]->Parents.size(); j++)
            if (n == n->Children[i]->Parents[j])
                n->Children[i]->Parents.erase(n->Children[i]->Parents.begin() + j);

    //remove the node from the list of nodes
    for (int i = 0; i < nodes.size(); i++)
        if (&nodes[i] == n)
            nodes.erase(nodes.begin() + i);
}

string RecurseNode(Node* n, string name)
{
    string code = ""; //string of python code
    if (n->Parents.size() != 0)
    {
        //if not all of the parents are processed, return nothing
        for (int i = 0; i < n->Parents.size(); i++)
            if (!n->Parents[i]->Processed)
                return code;
        if (!n->Processed)
        {
            if (n->Parents.size() > 1)
            {
                string mergename = "";
                string command1 = " = concatenate([";
                string command2 = "], axis=-1)\n";
                string tensors = "";
                for (int i = 0; i < n->Parents.size(); i++)
                {
                    if (n->Parents[i]->Type.compare("image") == 0 && n->Type.compare("flat") == 0)
                    {
                        code += n->Parents[i]->Name + "_flat" + " = Flatten()(" + n->Parents[i]->Name + ")\n\t";
                        if (i < n->Parents.size() - 1)
                            tensors += n->Parents[i]->Name + "_flat" + ", ";
                        else
                            tensors += n->Parents[i]->Name + "_flat";
                    }
                    else
                    {
                        if (i < n->Parents.size() - 1)
                            tensors += n->Parents[i]->Name + ", ";
                        else
                            tensors += n->Parents[i]->Name;
                    }
                    mergename += n->Parents[i]->Name;
                }
                code += mergename + command1 + tensors + command2 + "\t";
                name = mergename;
            }
            else
            {
                if (n->Parents[0]->Type.compare("image") == 0 && n->Type.compare("flat") == 0)
                {
                    code += n->Parents[0]->Name + "_flat" + " = Flatten()(" + n->Parents[0]->Name + ")\n\t";
                    name = n->Parents[0]->Name + "_flat";
                }
            }
            code += n->Command + "(" + name + ")\n\t";
            n->Processed = true;
        }
    }
    for (int i = 0; i < n->Children.size(); i++)
        code += RecurseNode(n->Children[i], n->Name);
    return code;
}

string GenerateModelCode()
{
    node_chain.clear();
    //string of python code
    string code = "from keras.layers import Input, Dense, Conv2D, Activation, Reshape, Flatten, concatenate, ELU, LeakyReLU, MaxPooling2D, AveragePooling2D\n";
    code += "from keras.models import Model\n";
    code += "import keras.backend as K\n";
    code += "import numpy as np\n\n";

    code += "K.set_image_dim_ordering('th')\n\n";

    code += "def AcaiModel():\n\t";

    //generate inputs
    for (int i = 0; i < nodes.size(); i++)
        //start by iterating through the input nodes
        if (nodes[i].Parents.size() == 0)
        {
            code += nodes[i].Command + "\t"; //create the input
            nodes[i].Processed = true;
        }

    //start by iterating through the input nodes and process each recursively
    for (int i = 0; i < nodes.size(); i++)
        if (nodes[i].Parents.size() == 0)
            code += RecurseNode(&nodes[i], code);

    for (int i = 0; i < nodes.size(); i++)
        nodes[i].Processed = false;

    //find the names of the inputs and outputs
    vector<string> inputs;
    vector<string> outputs;
    for (int i = 0; i < nodes.size(); i++)
        if (nodes[i].Parents.size() == 0)
            inputs.push_back(nodes[i].Name);
        else if (nodes[i].Children.size() == 0)
            outputs.push_back(nodes[i].Name);

    if (inputs.size() == 0)
        cout << "Error! No input nodes!" << endl;

    //write the model declaration code
    string inputString = "";
    string outputString = "";
    if (inputs.size() > 1)
    {
        inputString = "[";
        for (int i = 0; i < inputs.size(); i++)
            if (i != inputs.size() - 1)
                inputString += inputs[i] + ", ";
            else
                inputString += inputs[i];
        inputString += "]";
    }
    else if (inputs.size() == 0)
        return "";
    else
        inputString = inputs[0];

    if (outputs.size() > 1)
    {
        outputString = "[";
        for (int i = 0; i < outputs.size(); i++)
            if (i != outputs.size() - 1)
                outputString += outputs[i] + ", ";
            else
                outputString += outputs[i];
        outputString += "]";
    }
    else if (outputs.size() == 0)
        return "";
    else
        outputString = outputs[0];
    code += "\n\tmodel = Model(inputs=" + inputString + ", outputs=" + outputString + ")\n\t";
    code += "model.compile(loss='mse', optimizer='adam', metrics=['accuracy'])\n\t";
    code += "return model";
    return code;
}

void ProcessChunk(std::string s)
{
	vector<std::string> chunks;
	string chunk;
	for (int i = 0; i < s.length(); i++)
	{
		if (s[i] == '+')
        {
			chunks.push_back(chunk);
			cout << chunk << endl;
			chunk = "";
        }
		else
			chunk += s[i];
        if (i == s.length() - 1)
        {
            chunks.push_back(chunk);
        }
	}

	for (int i = 0; i < chunks.size(); i++)
    {
        if (chunks[i].find('x') != string::npos)
        {
            cout << "Processing conv input" << endl;
            //process conv input
            string dim = "";
            vector<int> dims;
            for (int j = 0; j < chunks[i].length(); j++)
            {
                if (chunks[i][j] == 'x')
                {
                    dims.push_back(atoi(dim.c_str()));
                    dim = "";
                }
                else
                    dim += chunks[i][j];
                if (j == chunks[i].size() - 1)
                {
                    dims.push_back(atoi(dim.c_str()));
                }
            }

            int pos_x = 500;
            int pos_y = 50;


            if (nodes.size() != 0)
            {
                sf::Rect<float> rect = nodes[nodes.size() - 1].Message.getGlobalBounds();
                pos_x = rect.left;
                pos_y = rect.top + 50;
                if (pos_y > 600)
                {
                    pos_x += 100;
                    pos_y = 50;
                }
            }

            Node n(sf::Vector2f(pos_x, pos_y), sf::String("Input " + to_string(dims[2])) + "x"
                   + sf::String(to_string(dims[0]) + "x" + to_string(dims[1])),
                   &font, 18, sf::Color::Black);
            n.Type = "image";
            n.Name = "x" + to_string(numInputs);
            n.Command = n.Name + " = Input(shape=(" + to_string(dims[2]) + ", " +
                        to_string(dims[0]) + ", " + to_string(dims[1]) + "))\n";
            nodes.push_back(n);
            cout << "Added conv input" << endl;
            numInputs++;
        }
        else if (chunks[i].find('c') != string::npos)
        {
            cout << "Processing conv layer " << chunks[i] << endl;
            //process conv layer
            string left = "";
            string right = "";
            for (int j = 0; j < chunks[i].length(); j++)
            {
                if (chunks[i][j] == 'c')
                {
                    left = right;
                    right = "";
                }
                else
                    right += chunks[i][j];
            }
            cout << left << " " << right << endl;
            unsigned int stride = 1;
            /*if (chunks[i].length() > i + 1)
            {
                if (chunks[i+1].find("stride") != string::npos)
                {
                    stride = atoi(chunks[i+1].substr(5, 3).c_str());
                }
                i++;
            }*/
            unsigned int numFilters = atoi(left.c_str());
            unsigned int kernelSize = atoi(right.c_str());

            int pos_x = 500;
            int pos_y = 50;
            if (nodes.size() != 0)
            {
                sf::Rect<float> rect = nodes[nodes.size() - 1].Message.getGlobalBounds();
                pos_x = rect.left;
                pos_y = rect.top + 50;
                if (pos_y > 600)
                {
                    pos_x += 100;
                    pos_y = 50;
                }
            }
            Node n(sf::Vector2f(pos_x, pos_y), sf::String(to_string(numFilters)) + "x"
                   + sf::String(to_string(kernelSize) + "x" + to_string(kernelSize)
                   + "conv " + to_string(stride) + "x" + to_string(stride) + "stride"),
                   &font, 18, sf::Color::Black);
            n.Type = "image";
            n.Name = "x" + to_string(nodes.size());
            n.Command = n.Name + " = Conv2D(" + to_string(numFilters) + ", ("
                        + to_string(kernelSize) + ", " + to_string(kernelSize) + ")"
                        + ", strides=(" + to_string(stride) + "," + to_string(stride) + "))";
            nodes.push_back(n);
            cout << "Added conv layer" << endl;

        }
        else if(chunks[i].find('n') != string::npos)
        {
            //process dense layer
            int dim = atoi(chunks[i].substr(0, chunks[i].length() - 1).c_str());
            sf::Rect<float> rect = nodes[nodes.size() -1 ].Message.getGlobalBounds();
            int pos_x = 500;
            int pos_y = 50;
            if (nodes.size() != 0)
            {
                sf::Rect<float> rect = nodes[nodes.size() - 1].Message.getGlobalBounds();
                pos_x = rect.left;
                pos_y = rect.top + 50;
                if (pos_y > 600)
                {
                    pos_x += 100;
                    pos_y = 50;
                }
            }
            Node n(sf::Vector2f(pos_x, pos_y), sf::String(to_string(dim)) + "nn",
                   &font, 18, sf::Color::Black);
                    n.Type = "flat";
                    n.Name = "x" + to_string(nodes.size());
                    n.Command = n.Name + " = Dense(" + to_string(dim) + ")";
            nodes.push_back(n);
            cout << "Added dense layer" << endl;
        }
        else if (chunks[i] == "relu" || chunks[i] == "tanh" || chunks[i] == "sigmoid"
                 || chunks[i] == "softmax" || chunks[i] == "elu" || chunks[i] == "leaky_relu")
        {
            sf::Rect<float> rect = nodes[nodes.size() -1 ].Message.getGlobalBounds();
            int pos_x = 500;
            int pos_y = 50;
            if (nodes.size() != 0)
            {
                sf::Rect<float> rect = nodes[nodes.size() - 1].Message.getGlobalBounds();
                pos_x = rect.left;
                pos_y = rect.top + 50;
                if (pos_y > 600)
                {
                    pos_x += 100;
                    pos_y = 50;
                }
            }
            string name = "Node " + to_string(nodes.size());
            string activation = chunks[i];
            Node n(sf::Vector2f(pos_x, pos_y), sf::String(activation), &font, 18, sf::Color::Black);
            n.Type = "activation";
            n.Name = "x" + to_string(nodes.size());
            n.Command = n.Name + " = Activation(\"" + activation + "\")";
            if (activation == "elu")
                n.Command = n.Name + " = ELU()";
            if (activation == "leaky_relu")
                n.Command = n.Name + " = LeakyReLU()";
            nodes.push_back(n);
            cout << "Added activation" << endl;
        }
        else if (chunks[i].find("ap") != string::npos || chunks[i].find("mp") != string::npos)
        {
            sf::Rect<float> rect = nodes[nodes.size() -1 ].Message.getGlobalBounds();
            int pos_x = 500;
            int pos_y = 50;
            if (nodes.size() != 0)
            {
                sf::Rect<float> rect = nodes[nodes.size() - 1].Message.getGlobalBounds();
                pos_x = rect.left;
                pos_y = rect.top + 50;
                if (pos_y > 600)
                {
                    pos_x += 100;
                    pos_y = 50;
                }
            }
            string name = "Node " + to_string(nodes.size());
            string poolSize = chunks[i].substr(2, chunks[i].length()).c_str();
            Node n(sf::Vector2f(pos_x, pos_y), sf::String(chunks[i]), &font, 18, sf::Color::Black);
            n.Type = "activation";
            n.Name = "x" + to_string(nodes.size());
            if (chunks[i].find("ap") != string::npos)
                n.Command = n.Name + " = MaxPooling2D(pool_size=(" + poolSize + ", " + poolSize + "))";
            else if (chunks[i].find("mp") != string::npos)
                n.Command = n.Name + " = AveragePooling2D(pool_size=(" + poolSize + ", " + poolSize + "))";
            nodes.push_back(n);
            cout << "Added activation" << endl;
        }
    }
}

void ParseModelString()
{
    string s = Prompt("Enter model string: ");
	std::string chunk;
	unsigned int numChunks = 0;
	for (int i = 0; i < s.length(); i++)
	{
		if (s[i] == '-')
        {
            if (chunk.find('*') != string::npos)
            {
                int numLayers = atoi(chunk.substr(0, chunk.find('*')).c_str());
                cout << numLayers << endl;
                chunk = chunk.substr(chunk.find('(') + 1, chunk.find(')') - chunk.find('(') - 1);
                for (int j = 0; j < numLayers; j++)
                {
                    ProcessChunk(chunk);
                    if (chunk.find('+') != string::npos)
                        numChunks++;
                    numChunks++;
                }
            }
			else
            {
                ProcessChunk(chunk);
                if (chunk.find('+') != string::npos)
                    numChunks++;
                numChunks++;
            }
			cout << chunk << endl;
			chunk = "";
        }
		else
			chunk += s[i];

        if (i == s.length() - 1)
        {
            if (chunk.find('*') != string::npos)
            {
                int numLayers = atoi(chunk.substr(0, chunk.find('*')).c_str());
                cout << numLayers << endl;
                chunk = chunk.substr(chunk.find('(') + 1, chunk.find(')') - chunk.find('(') - 1);
                for (int j = 0; j < numLayers; j++)
                {
                    ProcessChunk(chunk);
                    if (chunk.find('+') != string::npos)
                        numChunks++;
                    numChunks++;
                }
            }
			else
            {
                ProcessChunk(chunk);
                if (chunk.find('+') != string::npos)
                    numChunks++;
                numChunks++;
            }
			cout << chunk << endl;
			chunk = "";
        }
	}

	for (int i = 0; i < numChunks - 1; i++)
    {
        node_chain.clear();
        node_chain.push_back(&nodes[nodes.size() - 2 - i]);
        node_chain.push_back(&nodes[nodes.size() - 1 - i]);
        LinkNodes();
    }
}

void PrintCode()
{
    node_chain.clear();
    string code = GenerateModelCode();
    //string path = getenv("HOME");
    string path = "";
    if (path[path.length() - 1] != '/')
        path += "/Desktop";
    else
        path += "Desktop";

    string savepath = Prompt("Path to save python file? [Default is " + path + "]: ");
    while  (true)
    {
        if (savepath.length() != 0)
        {
            if (savepath[savepath.length() - 1] != '/')
                savepath += "/model.py";
            else
                savepath += "model.py";

            ofstream f;
            f.open(savepath);
            f << code;
            f.close();

            ifstream test(savepath);

            if (!test.good())
                savepath = Prompt("Invalid path! Path to save python file? [Default is " + path + "]: ");
            else
                break;
        }
        else
        {
            ofstream f;
            f.open(path + "/model.py");
            f << code;
            f.close();
            break;
        }

    }
}

void Train()
{
    string code = GenerateModelCode();

    //create model checkpointer
    code += "\nfrom keras.callbacks import ModelCheckpoint\n";
    code += "checkpointer = ModelCheckpoint(filepath='weights.hdf5', verbose=1, save_best_only=True)\n";

    //get the desired working path from the user
    string path = "";
    cout << "Save path?: ";
    cin >> path;
    if (path[path.length() - 1] != '/')
        path += "/";

    //get number of data sample
    int numTrainSamples = 0;
    int numTestSamples = 0;

    cout << "How many train data samples are there?: ";
    cin >> numTrainSamples;

    cout << "How many test data samples are there?: ";
    cin >> numTestSamples;

    //save the model architecture
    code += "\nmodel.save('" + path + "model.h5')\n";

    vector<string> inputs;
    vector<string> outputs;
    for (int i = 0; i < nodes.size(); i++)
        if (nodes[i].Parents.size() == 0)
        {
            string filename = "";
            cout << "Specify .dat train file for " + nodes[i].Message.getString().toAnsiString() << " input: ";
            cin >> filename;
            code += nodes[i].Name + "_trainx = np.fromfile(\"" + path + filename + "\", np.float32)\n";
            code += nodes[i].Name + "_trainx = np.reshape(" + nodes[i].Name + "_trainx, ("
            + to_string(numTrainSamples) + ", -1))\n";

            cout << "Specify .dat test file for " + nodes[i].Message.getString().toAnsiString() << " input: ";
            cin >> filename;
            code += nodes[i].Name + "_testx = np.fromfile(\"" + path + filename + "\", np.float32)\n";
            code += nodes[i].Name + "_testx = np.reshape(" + nodes[i].Name + "_testx, ("
            + to_string(numTestSamples) + ", -1))\n";
            inputs.push_back(nodes[i].Name);
        }

    for (int i = 0; i < nodes.size(); i++)
        if (nodes[i].Children.size() == 0)
        {
            string filename = "";
            cout << "Specify .dat train file for " + nodes[i].Message.getString().toAnsiString() << " output: ";
            cin >> filename;
            code += nodes[i].Name + "_trainy = np.fromfile(\"" + path + filename + "\", np.float32)\n";
            code += nodes[i].Name + "_trainy = np.reshape(" + nodes[i].Name + "_trainy, ("
            + to_string(numTrainSamples) + ", -1))";

            cout << "Specify .dat test file for " + nodes[i].Message.getString().toAnsiString() << " output: ";
            cin >> filename;
            code += nodes[i].Name + "_testy = np.fromfile(\"" + path + filename + "\", np.float32)\n";
            code += nodes[i].Name + "_testy = np.reshape(" + nodes[i].Name + "_testy, ("
            + to_string(numTestSamples) + ", -1))";

            outputs.push_back(nodes[i].Name);
        }

    //write the train and test input and output strings
    string trainInputString = "";
    string testInputString = "";
    if (inputs.size() > 1)
    {
        trainInputString = "[";
        for (int i = 0; i < inputs.size(); i++)
            if (i != inputs.size() - 1)
            {
                trainInputString += inputs[i] + "_trainx, ";
                testInputString += inputs[i] + "_testx, ";
            }
            else
            {
                trainInputString += inputs[i] + "_trainx]";
                testInputString += inputs[i] + "_testx]";
            }
    }
    else
    {
        trainInputString = inputs[0] + "_trainx";
        testInputString = inputs[0] + "_testx";
    }

    string trainOutputString = "";
    string testOutputString = "";
    if (outputs.size() > 1)
    {
        trainOutputString = "[";
        for (int i = 0; i < outputs.size(); i++)
            if (i != outputs.size() - 1)
            {
                trainOutputString += outputs[i] + "_trainy, ";
                testOutputString += outputs[i] + "_testy, ";
            }
            else
            {
                trainOutputString += outputs[i] + "_trainy]";
                testOutputString += outputs[i] + "_testy]";
            }
    }
    else
    {
        trainOutputString = outputs[0] + "_trainy";
        testOutputString = outputs[0] + "_testy";
    }

    code += "\nmodel.fit(" + trainInputString + ", " + trainOutputString
    + ", batch_size=128, epochs=100, verbose=1, validation_data=("
    + testInputString + ", " + testOutputString + "))\n";
    cout << code << endl;
    system(("python -m " + code).c_str());
}

int main(int argc, char* argv[])
{
    buttons.push_back(Button(sf::Vector2f(50, 10), "Create New Flat Input", &font,
                             18, sf::Color::Black, CreateFlatInput));
    buttons.push_back(Button(sf::Vector2f(50, 110), "Create New Image Input", &font,
                             18, sf::Color::Black, CreateImageInput));
    buttons.push_back(Button(sf::Vector2f(50, 210), "Create Fully Connected Layer", &font,
                             18, sf::Color::Black, CreateFCL));
    buttons.push_back(Button(sf::Vector2f(50, 310), "Create Convolutional Layer", &font,
                             18, sf::Color::Black, CreateConv));
    buttons.push_back(Button(sf::Vector2f(50, 410), "Create Activation", &font,
                             18, sf::Color::Black, CreateActivation));
    buttons.push_back(Button(sf::Vector2f(50, 510), "Create or Remove Link",
                             &font, 18, sf::Color::Black, LinkNodes));
    buttons.push_back(Button(sf::Vector2f(50, 610), "Generate Model Code",
                             &font, 18, sf::Color::Black, PrintCode));
    buttons.push_back(Button(sf::Vector2f(50, 710), "Input Model String",
                             &font, 18, sf::Color::Black, ParseModelString));
    //create render window
    sf::String title_string = "Acai";
    sf::RenderWindow Window(sf::VideoMode(width, height), title_string);
    Window.setFramerateLimit(120);
    window = &Window;

    //load font
    if (!font.loadFromFile("./Arial.ttf"))
    {
        cout << "Couldn't load font!" << endl;
        return -1;
    }

    //main update loop
    while (window->isOpen())
    {
        //check for events
        sf::Event event;
        while (window->pollEvent(event))
        {
            //close if close signal detected
            if (event.type == sf::Event::Closed)
                window->close();
            //delete selected nodes if backspace is pressed
            else if(event.type == sf::Event::KeyPressed)
            {
                if(event.key.code == sf::Keyboard::BackSpace)
                {
                    for (int i = 0; i < node_chain.size(); i++)
                        RemoveNode(node_chain[i]);
                    node_chain.clear();
                }
                else if (event.key.code == sf::Keyboard::Return)
                    LinkNodes();
            }
        }

        //Update and draw
        Update();
        Draw();
        window->display();
    }
}
