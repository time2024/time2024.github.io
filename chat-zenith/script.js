document.addEventListener('DOMContentLoaded', function() {
    // 配置marked.js - 用于渲染Markdown格式的回复
    marked.setOptions({
        highlight: function(code, lang) {
            const language = hljs.getLanguage(lang) ? lang : 'plaintext';
            return hljs.highlight(code, { language }).value;
        },
        breaks: true,
        gfm: true
    });

    const chatHistory = document.getElementById('chat-history');
    const userInput = document.getElementById('user-input');
    const sendBtn = document.getElementById('send-btn');
    const clearBtn = document.getElementById('clear-btn');
    
    // API端点配置
    const API_URL = 'https://api.smallgoodgood.top/chat';
    
    // 对话历史记录，用于保持上下文
    let conversationHistory = [];
    
    /**
     * 发送消息的主函数
     */
    async function sendMessage() {
        const message = userInput.value.trim();
        if (!message) return;
        
        sendBtn.disabled = true;
        userInput.disabled = true;
        
        addMessageToChat('user', message);
        userInput.value = '';
        
        const thinkingMsgId = showThinkingMessage();
        
        try {
            console.log('正在发送请求到:', API_URL);
            const response = await callLangchainAPI(message);
            console.log('收到响应:', response);
            
            removeThinkingMessage(thinkingMsgId);
            
            if (response && response.choices && response.choices[0]) {
                const aiMessage = response.choices[0].message;
                if (aiMessage && aiMessage.content) {
                    const aiResponse = aiMessage.content;
                    addMessageToChat('assistant', aiResponse, true);
                    
                    conversationHistory.push(
                        { role: 'user', content: message },
                        { role: 'assistant', content: aiResponse }
                    );
                } else {
                    console.error('响应格式不完整，message对象:', aiMessage);
                    throw new Error('API响应格式不正确：缺少message.content');
                }
            } else {
                console.error('响应格式错误，完整响应:', response);
                throw new Error('API响应格式不正确：缺少choices数组');
            }
        } catch (error) {
            removeThinkingMessage(thinkingMsgId);
            console.error('处理消息时出错:', error);
            
            let errorMessage = '博士...... 我好像发生了故障...... 我好想你呀 博士......';
            addMessageToChat('assistant', errorMessage);
        } finally {
            sendBtn.disabled = false;
            userInput.disabled = false;
            userInput.focus();
        }
    }
    
    /**
     * 调用Langchain API的函数
     */
    async function callLangchainAPI(message) {
        try {
            const requestBody = {
                messages: [
                    ...conversationHistory,
                    { role: 'user', content: message }
                ]
            };
            
            console.log('发送请求体:', requestBody);
            
            const response = await fetch(API_URL, {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                },
                body: JSON.stringify(requestBody)
            });
            
            console.log('响应状态:', response.status, response.statusText);
            
            if (!response.ok) {
                let errorMessage = `请求失败: ${response.status} ${response.statusText}`;
                try {
                    const errorData = await response.json();
                    if (errorData.error) {
                        errorMessage = errorData.error;
                    }
                } catch (e) {
                    console.error('无法解析错误响应:', e);
                }
                throw new Error(errorMessage);
            }
            
            let data;
            try {
                const responseText = await response.text();
                console.log('原始响应文本:', responseText);
                data = JSON.parse(responseText);
                console.log('解析后的JSON:', data);
            } catch (parseError) {
                console.error('JSON解析失败:', parseError);
                throw new Error('服务器响应格式错误，无法解析JSON');
            }
            
            return data;
            
        } catch (error) {
            console.error('API调用失败:', error);
            throw error;
        }
    }
    
    /**
     * 添加消息到聊天界面
     * 结构与HTML中的欢迎消息一致
     */
    function addMessageToChat(role, content, isMarkdown = false) {
        const messageDiv = document.createElement('div');
        
        if (role === 'assistant') {
            // AI消息样式 - 与HTML欢迎消息结构一致
            messageDiv.className = 'message ai-message';

            // 添加猫咪头像
            const avatar = document.createElement('img');
            avatar.src = 'https://smallgoodgood.top/images/23.jpg';
            avatar.className = 'cat-avatar';
            messageDiv.appendChild(avatar);

            // 内容容器（包含文本和时间戳）
            const contentDiv = document.createElement('div');
            
            if (isMarkdown) {
                // 处理数学公式并渲染Markdown
                let mathContent = content
                    .replace(/\\\((.+?)\\\)/g, function(match, p1) {
                        return '$' + p1 + '$';
                    })
                    .replace(/\\\[(.+?)\\\]/g, function(match, p1) {
                        return '$$' + p1 + '$$';
                    });
                contentDiv.innerHTML = marked.parse(mathContent);
            } else {
                // 非markdown内容也用span包裹，保持与思考消息结构一致
                const textSpan = document.createElement('span');
                textSpan.textContent = content;
                contentDiv.appendChild(textSpan);
            }
            
            // 时间戳放在内容div内部
            const messageTime = document.createElement('div');
            messageTime.className = 'message-time';
            messageTime.textContent = new Date().toLocaleTimeString();
            contentDiv.appendChild(messageTime);
            
            messageDiv.appendChild(contentDiv);
        } else {
            // 用户消息样式
            messageDiv.className = 'message user-message';
            const messageContent = document.createElement('div');
            messageContent.textContent = content;
            messageDiv.appendChild(messageContent);
            
            // 用户消息的时间戳
            const messageTime = document.createElement('div');
            messageTime.className = 'message-time';
            messageTime.textContent = new Date().toLocaleTimeString();
            messageDiv.appendChild(messageTime);
        }

        // 添加到聊天历史
        chatHistory.appendChild(messageDiv);
        chatHistory.scrollTop = chatHistory.scrollHeight;

        // 高亮代码块（如果有）
        if (isMarkdown && role === 'assistant') {
            messageDiv.querySelectorAll('pre code').forEach((block) => {
                hljs.highlightElement(block);
            });
        }

        // 渲染数学公式
        if (window.MathJax && typeof MathJax.typesetPromise === 'function') {
            MathJax.typesetPromise([messageDiv]).catch(err => {
                console.error('MathJax渲染错误:', err);
            });
        }
    }

    /**
     * 显示AI思考中的动画提示
     * 结构与HTML欢迎消息一致
     */
    function showThinkingMessage() {
        const messageDiv = document.createElement('div');
        messageDiv.className = 'message ai-message';
        messageDiv.id = 'thinking-message';

        // 头像
        const avatar = document.createElement('img');
        avatar.src = 'https://smallgoodgood.top/images/23.jpg';
        avatar.className = 'cat-avatar';
        messageDiv.appendChild(avatar);

        // 内容容器
        const contentDiv = document.createElement('div');
        
        const thinkingText = document.createElement('span');
        thinkingText.textContent = '正在思考...';
        contentDiv.appendChild(thinkingText);
        
        // 移除loading动画，避免高度不一致
        // 改用CSS动画的省略号效果
        const loadingDots = document.createElement('span');
        loadingDots.className = 'loading-dots';
        contentDiv.appendChild(loadingDots);

        // 时间戳放在内容div内部
        const messageTime = document.createElement('div');
        messageTime.className = 'message-time';
        messageTime.textContent = new Date().toLocaleTimeString();
        contentDiv.appendChild(messageTime);

        messageDiv.appendChild(contentDiv);

        chatHistory.appendChild(messageDiv);
        chatHistory.scrollTop = chatHistory.scrollHeight;

        return 'thinking-message';
    }
    
    /**
     * 移除思考提示
     */
    function removeThinkingMessage(messageId) {
        const thinkingMsg = document.getElementById(messageId);
        if (thinkingMsg) {
            thinkingMsg.remove();
        }
    }
    
    /**
     * 清空聊天历史
     */
    function clearChatHistory() {
        const welcomeMessage = chatHistory.querySelector('.ai-message');
        chatHistory.innerHTML = '';
        if (welcomeMessage) {
            chatHistory.appendChild(welcomeMessage.cloneNode(true));
        }
        conversationHistory = [];
        console.log('聊天历史已清空');
    }
    
    // ===== 事件监听器 =====
    sendBtn.addEventListener('click', sendMessage);
    
    userInput.addEventListener('keydown', function(e) {
        if (e.key === 'Enter' && !e.shiftKey) {
            e.preventDefault();
            sendMessage();
        }
    });
    
    clearBtn.addEventListener('click', clearChatHistory);
    
    userInput.setAttribute('maxlength', '500');

    let charCountTip = document.createElement('div');
    charCountTip.style.position = 'absolute';
    charCountTip.style.left = '16px';
    charCountTip.style.bottom = '12px';
    charCountTip.style.fontSize = '12px';
    charCountTip.style.color = '#888';
    charCountTip.style.zIndex = '2';
    userInput.parentNode.appendChild(charCountTip);

    function updateCharCount() {
        const len = userInput.value.length;
        charCountTip.textContent = `${len}/500`;
        if (len >= 500) {
            charCountTip.style.color = '#e74c3c';
        } else {
            charCountTip.style.color = '#888';
        }
    }
    
    userInput.addEventListener('input', updateCharCount);
    updateCharCount();

    userInput.focus();
    
    console.log('Chat Zenith 前端已加载');
    console.log('API端点:', API_URL);
});
